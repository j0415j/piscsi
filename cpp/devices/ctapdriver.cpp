//---------------------------------------------------------------------------
//
//	SCSI Target Emulator PiSCSI
//	for Raspberry Pi
//
//	Powered by XM6 TypeG Technology.
//	Copyright (C) 2016-2020 GIMONS
//	Copyright (C) akuker
//
//---------------------------------------------------------------------------

#include "shared/log.h"
#include "shared/piscsi_util.h"
#include "shared/piscsi_exceptions.h"
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include "ctapdriver.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <sstream>

#ifdef __linux__
#include <sys/epoll.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/sockios.h>
#endif

using namespace std;
using namespace piscsi_util;

//---------------------------------------------------------------------------
//
//	Initialization
//
//---------------------------------------------------------------------------
static bool br_setif(int br_socket_fd, const char* bridgename, const char* ifname, bool add) {
#ifndef __linux__
	return false;
#else
	ifreq ifr;
	ifr.ifr_ifindex = if_nametoindex(ifname);
	if (ifr.ifr_ifindex == 0) {
		LOGERROR("Can't if_nametoindex %s: %s", ifname, strerror(errno))
		return false;
	}
	strncpy(ifr.ifr_name, bridgename, IFNAMSIZ - 1);
	if (ioctl(br_socket_fd, add ? SIOCBRADDIF : SIOCBRDELIF, &ifr) < 0) {
		LOGERROR("Can't ioctl %s: %s", add ? "SIOCBRADDIF" : "SIOCBRDELIF", strerror(errno))
		return false;
	}
	return true;
#endif
}

CTapDriver::~CTapDriver()
{
	if (m_hTAP != -1) {
		if (int br_socket_fd; (br_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
			LOGERROR("Can't open bridge socket: %s", strerror(errno))
		} else {
			LOGDEBUG("brctl delif %s piscsi0", BRIDGE_NAME)
			if (!br_setif(br_socket_fd, BRIDGE_NAME, "piscsi0", false)) { //NOSONAR No exception is raised here
				LOGWARN("Warning: Removing piscsi0 from the bridge failed.")
				LOGWARN("You may need to manually remove the piscsi0 tap device from the bridge")
			}
			close(br_socket_fd);
		}

		// Release TAP defice
		close(m_hTAP);
	}

	if (m_pcap_dumper != nullptr) {
		pcap_dump_close(m_pcap_dumper);
	}

	if (m_pcap != nullptr) {
		pcap_close(m_pcap);
	}
}

static bool ip_link(int fd, const char* ifname, bool up) {
#ifndef __linux__
	return false;
#else
	ifreq ifr;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1); // Need to save room for null terminator
	int err = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (err) {
		LOGERROR("Can't ioctl SIOCGIFFLAGS: %s", strerror(errno))
		return false;
	}
	ifr.ifr_flags &= ~IFF_UP;
	if (up) {
		ifr.ifr_flags |= IFF_UP;
	}
	err = ioctl(fd, SIOCSIFFLAGS, &ifr);
	if (err) {
		LOGERROR("Can't ioctl SIOCSIFFLAGS: %s", strerror(errno))
		return false;
	}
	return true;
#endif
}

static bool is_interface_up(string_view interface) {
	string file = "/sys/class/net/";
	file += interface;
	file += "/carrier";

	bool status = true;
	FILE *fp = fopen(file.c_str(), "r");
	if (!fp || fgetc(fp) != '1') {
		status = false;
	}

	if (fp) {
		fclose(fp);
	}

	return status;
}

bool CTapDriver::Init(const unordered_map<string, string>& const_params)
{
#ifndef __linux__
	return false;
#else
	unordered_map<string, string> params = const_params;
	stringstream s(params["interface"]);
	string interface;
	while (getline(s, interface, ',')) {
		interfaces.push_back(interface);
	}
	inet = params["inet"];

	LOGTRACE("Opening tap device")
	// TAP device initilization
	if ((m_hTAP = open("/dev/net/tun", O_RDWR)) < 0) {
		LOGERROR("Can't open tun: %s", strerror(errno))
		return false;
	}

	LOGTRACE("Opened tap device %d", m_hTAP)

	// IFF_NO_PI for no extra packet information
	ifreq ifr = {};
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	string dev = "piscsi0";
	strncpy(ifr.ifr_name, dev.c_str(), IFNAMSIZ - 1);

	LOGTRACE("Going to open %s", ifr.ifr_name)

	int ret = ioctl(m_hTAP, TUNSETIFF, (void *)&ifr);
	if (ret < 0) {
		LOGERROR("Can't ioctl TUNSETIFF: %s", strerror(errno))

		close(m_hTAP);
		return false;
	}

	LOGTRACE("Return code from ioctl was %d", ret)

	const int ip_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (ip_fd < 0) {
		LOGERROR("Can't open ip socket: %s", strerror(errno))

		close(m_hTAP);
		return false;
	}

	const int br_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (br_socket_fd < 0) {
		LOGERROR("Can't open bridge socket: %s", strerror(errno))

		close(m_hTAP);
		close(ip_fd);
		return false;
	}

	// Check if the bridge has already been created
	string sys_file = "/sys/class/net/";
	sys_file += BRIDGE_NAME;
	if (access(sys_file.c_str(), F_OK)) {
		LOGINFO("%s is not yet available", BRIDGE_NAME)

		LOGTRACE("Checking which interface is available for creating the bridge")

		string bridge_interface;
		for (const string& iface : interfaces) {
			if (is_interface_up(iface)) {
				LOGTRACE("%s", string("Interface " + iface + " is up").c_str())

				bridge_interface = iface;
				break;
			}
			else {
				LOGTRACE("%s", string("Interface " + iface + " is not available or is not up").c_str())
			}
		}

		if (bridge_interface.empty()) {
			LOGERROR("No interface is up, not creating bridge")
			return false;
		}

		LOGINFO("Creating %s for interface %s", BRIDGE_NAME, bridge_interface.c_str())

		if (bridge_interface == "eth0") {
			LOGTRACE("brctl addbr %s", BRIDGE_NAME)

			if (ioctl(br_socket_fd, SIOCBRADDBR, BRIDGE_NAME) < 0) {
				LOGERROR("Can't ioctl SIOCBRADDBR: %s", strerror(errno))

				close(m_hTAP);
				close(ip_fd);
				close(br_socket_fd);
				return false;
			}

			LOGTRACE("brctl addif %s %s", BRIDGE_NAME, bridge_interface.c_str())

			if (!br_setif(br_socket_fd, BRIDGE_NAME, bridge_interface.c_str(), true)) {
				close(m_hTAP);
				close(ip_fd);
				close(br_socket_fd);
				return false;
			}
		}
		else {
			string address = inet;
			string netmask = "255.255.255.0"; //NOSONAR This hardcoded IP address is safe
			if (size_t separatorPos = inet.find('/'); separatorPos != string::npos) {
				address = inet.substr(0, separatorPos);

				int m;
				if (!GetAsUnsignedInt(inet.substr(separatorPos + 1), m) || m < 8 || m > 32) {
					LOGERROR("Invalid CIDR netmask notation '%s'", inet.substr(separatorPos + 1).c_str())

					close(m_hTAP);
					close(ip_fd);
					close(br_socket_fd);
					return false;
				}

				// long long is required for compatibility with 32 bit platforms
				const auto mask = (long long)(pow(2, 32) - (1 << (32 - m)));
				netmask = to_string((mask >> 24) & 0xff) + '.' + to_string((mask >> 16) & 0xff) + '.' +
						to_string((mask >> 8) & 0xff) + '.' + to_string(mask & 0xff);

			}

			LOGTRACE("brctl addbr %s", BRIDGE_NAME)

			if (ioctl(br_socket_fd, SIOCBRADDBR, BRIDGE_NAME) < 0) {
				LOGERROR("Can't ioctl SIOCBRADDBR: %s", strerror(errno))

				close(m_hTAP);
				close(ip_fd);
				close(br_socket_fd);
				return false;
			}

			ifreq ifr_a;
			ifr_a.ifr_addr.sa_family = AF_INET;
			strncpy(ifr_a.ifr_name, BRIDGE_NAME, IFNAMSIZ);
			if (auto addr = (sockaddr_in*)&ifr_a.ifr_addr;
				inet_pton(AF_INET, address.c_str(), &addr->sin_addr) != 1) {
				LOGERROR("Can't convert '%s' into a network address: %s", address.c_str(), strerror(errno))

				close(m_hTAP);
				close(ip_fd);
				close(br_socket_fd);
				return false;
			}

			ifreq ifr_n;
			ifr_n.ifr_addr.sa_family = AF_INET;
			strncpy(ifr_n.ifr_name, BRIDGE_NAME, IFNAMSIZ);
			if (auto mask = (sockaddr_in*)&ifr_n.ifr_addr;
				inet_pton(AF_INET, netmask.c_str(), &mask->sin_addr) != 1) {
				LOGERROR("Can't convert '%s' into a netmask: %s", netmask.c_str(), strerror(errno))

				close(m_hTAP);
				close(ip_fd);
				close(br_socket_fd);
				return false;
			}

			LOGTRACE("ip address add %s dev %s", inet.c_str(), BRIDGE_NAME)

			if (ioctl(ip_fd, SIOCSIFADDR, &ifr_a) < 0 || ioctl(ip_fd, SIOCSIFNETMASK, &ifr_n) < 0) {
				LOGERROR("Can't ioctl SIOCSIFADDR or SIOCSIFNETMASK: %s", strerror(errno))

				close(m_hTAP);
				close(ip_fd);
				close(br_socket_fd);
				return false;
			}
		}

		LOGTRACE("ip link set dev %s up", BRIDGE_NAME)

		if (!ip_link(ip_fd, BRIDGE_NAME, true)) {
			close(m_hTAP);
			close(ip_fd);
			close(br_socket_fd);
			return false;
		}
	}
	else
	{
		LOGINFO("%s is already available", BRIDGE_NAME)
	}

	LOGTRACE("ip link set piscsi0 up")

	if (!ip_link(ip_fd, "piscsi0", true)) {
		close(m_hTAP);
		close(ip_fd);
		close(br_socket_fd);
		return false;
	}

	LOGTRACE("brctl addif %s piscsi0", BRIDGE_NAME)

	if (!br_setif(br_socket_fd, BRIDGE_NAME, "piscsi0", true)) {
		close(m_hTAP);
		close(ip_fd);
		close(br_socket_fd);
		return false;
	}

	// Get MAC address
	LOGTRACE("Getting the MAC address")

	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(m_hTAP, SIOCGIFHWADDR, &ifr) < 0) {
		LOGERROR("Can't ioctl SIOCGIFHWADDR: %s", strerror(errno))

		close(m_hTAP);
		close(ip_fd);
		close(br_socket_fd);
		return false;
	}
	LOGTRACE("Got the MAC")

	// Save MAC address
	memcpy(m_MacAddr.data(), ifr.ifr_hwaddr.sa_data, m_MacAddr.size());

	close(ip_fd);
	close(br_socket_fd);

	LOGINFO("Tap device %s created", ifr.ifr_name)

	return true;
#endif
}

void CTapDriver::OpenDump(const string& path) {
	if (m_pcap == nullptr) {
		m_pcap = pcap_open_dead(DLT_EN10MB, 65535);
	}
	if (m_pcap_dumper != nullptr) {
		pcap_dump_close(m_pcap_dumper);
	}
	m_pcap_dumper = pcap_dump_open(m_pcap, path.c_str());
	if (m_pcap_dumper == nullptr) {
		LOGERROR("Can't open pcap file: %s", pcap_geterr(m_pcap))
		throw io_exception("Can't open pcap file");
	}

	LOGTRACE("%s Opened %s for dumping", __PRETTY_FUNCTION__, path.c_str())
}

bool CTapDriver::Enable() const
{
	const int fd = socket(PF_INET, SOCK_DGRAM, 0);
	LOGDEBUG("%s: ip link set piscsi0 up", __PRETTY_FUNCTION__)
	const bool result = ip_link(fd, "piscsi0", true);
	close(fd);
	return result;
}

bool CTapDriver::Disable() const
{
	const int fd = socket(PF_INET, SOCK_DGRAM, 0);
	LOGDEBUG("%s: ip link set piscsi0 down", __PRETTY_FUNCTION__)
	const bool result = ip_link(fd, "piscsi0", false);
	close(fd);
	return result;
}

void CTapDriver::Flush()
{
	LOGTRACE("%s", __PRETTY_FUNCTION__)
	while (PendingPackets()) {
		array<uint8_t, ETH_FRAME_LEN> m_garbage_buffer;
		(void)Receive(m_garbage_buffer.data());
	}
}

void CTapDriver::GetMacAddr(uint8_t *mac) const
{
	assert(mac);

	memcpy(mac, m_MacAddr.data(), m_MacAddr.size());
}

//---------------------------------------------------------------------------
//
//	Receive
//
//---------------------------------------------------------------------------
bool CTapDriver::PendingPackets() const
{
	assert(m_hTAP != -1);

	// Check if there is data that can be received
	pollfd fds;
	fds.fd = m_hTAP;
	fds.events = POLLIN | POLLERR;
	fds.revents = 0;
	poll(&fds, 1, 0);
	LOGTRACE("%s %u revents", __PRETTY_FUNCTION__, fds.revents)
	if (!(fds.revents & POLLIN)) {
		return false;
	} else {
		return true;
	}
}

// See https://stackoverflow.com/questions/21001659/crc32-algorithm-implementation-in-c-without-a-look-up-table-and-with-a-public-li
uint32_t CTapDriver::Crc32(const uint8_t *buf, int length) {
   uint32_t crc = 0xffffffff;
   for (int i = 0; i < length; i++) {
      crc ^= buf[i];
      for (int j = 0; j < 8; j++) {
         const uint32_t mask = -(static_cast<int>(crc) & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
}

int CTapDriver::Receive(uint8_t *buf)
{
	assert(m_hTAP != -1);

	// Check if there is data that can be received
	if (!PendingPackets()) {
		return 0;
	}

	// Receive
	auto dwReceived = static_cast<uint32_t>(read(m_hTAP, buf, ETH_FRAME_LEN));
	if (dwReceived == static_cast<uint32_t>(-1)) {
		LOGWARN("%s Error occured while receiving a packet", __PRETTY_FUNCTION__)
		return 0;
	}

	// If reception is enabled
	if (dwReceived > 0) {
		// We need to add the Frame Check Status (FCS) CRC back onto the end of the packet.
		// The Linux network subsystem removes it, since most software apps shouldn't ever
		// need it.
		const int crc = Crc32(buf, dwReceived);

		buf[dwReceived + 0] = (uint8_t)((crc >> 0) & 0xFF);
		buf[dwReceived + 1] = (uint8_t)((crc >> 8) & 0xFF);
		buf[dwReceived + 2] = (uint8_t)((crc >> 16) & 0xFF);
		buf[dwReceived + 3] = (uint8_t)((crc >> 24) & 0xFF);

		LOGDEBUG("%s CRC is %08X - %02X %02X %02X %02X\n", __PRETTY_FUNCTION__, crc, buf[dwReceived+0], buf[dwReceived+1], buf[dwReceived+2], buf[dwReceived+3])

		// Add FCS size to the received message size
		dwReceived += 4;
	}

	if (m_pcap_dumper != nullptr) {
		pcap_pkthdr h = {
			.ts = {},
			.caplen = dwReceived,
			.len = dwReceived
		};
		gettimeofday(&h.ts, nullptr);
		pcap_dump((u_char*)m_pcap_dumper, &h, buf);
		LOGTRACE("%s Dumped %d byte packet (first byte: %02x last byte: %02x)", __PRETTY_FUNCTION__, (unsigned int)dwReceived, buf[0], buf[dwReceived-1])
	}

	// Return the number of bytes
	return dwReceived;
}

int CTapDriver::Send(const uint8_t *buf, int len)
{
	assert(m_hTAP != -1);

	if (m_pcap_dumper != nullptr) {
		pcap_pkthdr h = {
			.ts = {},
			.caplen = (bpf_u_int32)len,
			.len = (bpf_u_int32)len,
		};
		gettimeofday(&h.ts, nullptr);
		pcap_dump((u_char*)m_pcap_dumper, &h, buf);
		LOGTRACE("%s Dumped %d byte packet (first byte: %02x last byte: %02x)", __PRETTY_FUNCTION__, (unsigned int)h.len, buf[0], buf[h.len-1])
	}

	// Start sending
	return static_cast<int>(write(m_hTAP, buf, len));
}
