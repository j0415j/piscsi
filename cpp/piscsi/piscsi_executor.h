//---------------------------------------------------------------------------
//
// SCSI Target Emulator PiSCSI
// for Raspberry Pi
//
// Copyright (C) 2021-2022 Uwe Seimet
//
//---------------------------------------------------------------------------

#pragma once

#include "spdlog/spdlog.h"
#include "shared/protobuf_serializer.h"
#include "piscsi/piscsi_response.h"
#include <unordered_set>
#include <unordered_map>

class PiscsiImage;
class DeviceFactory;
class ControllerManager;
class PrimaryDevice;
class CommandContext;

using namespace spdlog;

class PiscsiExecutor
{
public:

	PiscsiExecutor(PiscsiImage& piscsi_image, ControllerManager& controller_manager)
		: piscsi_image(piscsi_image), controller_manager(controller_manager) {}
	~PiscsiExecutor() = default;

	unordered_set<int> GetReservedIds() const { return reserved_ids; }

	bool ProcessDeviceCmd(const CommandContext&, const PbDeviceDefinition&, const PbCommand&, bool);
	bool ProcessCmd(const CommandContext&, const PbCommand&);
	bool SetLogLevel(const string&) const;
	bool Start(PrimaryDevice&, bool) const;
	bool Stop(PrimaryDevice&, bool) const;
	bool Eject(PrimaryDevice&, bool) const;
	bool Protect(PrimaryDevice&, bool) const;
	bool Unprotect(PrimaryDevice&, bool) const;
	bool Attach(const CommandContext&, const PbDeviceDefinition&, bool);
	bool Insert(const CommandContext&, const PbDeviceDefinition&, const shared_ptr<PrimaryDevice>&, bool) const;
	bool Detach(const CommandContext&, const shared_ptr<PrimaryDevice>&, bool) const;
	void DetachAll();
	bool ShutDown(const CommandContext&, const string&);
	string SetReservedIds(string_view);
	bool ValidateImageFile(const CommandContext&, StorageDevice&, const string&, string&) const;
	void PrintCommand(const PbCommand&, const PbDeviceDefinition&, bool) const;
	string ValidateLunSetup(const PbCommand&) const;
	bool VerifyExistingIdAndLun(const CommandContext&, int, int) const;
	shared_ptr<PrimaryDevice> CreateDevice(const CommandContext&, const PbDeviceType, int, const string&) const;
	bool SetSectorSize(const CommandContext&, shared_ptr<PrimaryDevice>, int) const;

	static bool ValidateOperationAgainstDevice(const CommandContext&, const PrimaryDevice&, const PbOperation&);
	static bool ValidateIdAndLun(const CommandContext&, int, int);
	static bool SetProductData(const CommandContext&, const PbDeviceDefinition&, PrimaryDevice&);

private:

	const PiscsiResponse piscsi_response;

	PiscsiImage& piscsi_image;

	ControllerManager& controller_manager;

	const DeviceFactory device_factory;

	const ProtobufSerializer serializer;

	unordered_set<int> reserved_ids;

	static inline const unordered_map<string, level::level_enum> log_level_mapping = {
			{ "trace", level::trace },
			{ "debug", level::debug },
			{ "info", level::info },
			{ "warn", level::warn },
			{ "err", level::err },
			{ "off", level::off }
	};
};
