//---------------------------------------------------------------------------
//
//	SCSI Target Emulator PiSCSI
//	for Raspberry Pi
//
//	Powered by XM6 TypeG Technology.
//	Copyright (C) 2016-2020 GIMONS
//	[ Host File System for the X68000 ]
//
//  Note: This functionality is specific to the X68000 operating system.
//        It is highly unlikely that this will work for other platforms.
//---------------------------------------------------------------------------

#pragma once

using TCHAR = char;

static const int FILEPATH_MAX = 260;

//---------------------------------------------------------------------------
//
//	Status code definitions
//
//---------------------------------------------------------------------------
static const int FS_INVALIDFUNC	=	0xFFFFFFFF;	///< Executed an invalid function
static const int FS_FILENOTFND =	0xFFFFFFFE;	///< The selected file can not be found
static const int FS_DIRNOTFND =		0xFFFFFFFD;	///< The selected directory can not be found
static const int FS_OVEROPENED =	0xFFFFFFFC;	///< There are too many files open
static const int FS_CANTACCESS =	0xFFFFFFFB;	///< Can not access the direcory or volume
static const int FS_NOTOPENED =		0xFFFFFFFA;	///< The selected handle is not opened
static const int FS_INVALIDMEM =	0xFFFFFFF9;	///< Memory management has been destroyed
static const int FS_OUTOFMEM =		0xFFFFFFF8;	///< Insufficient memory for execution
static const int FS_INVALIDPTR =	0xFFFFFFF7;	///< Selected an invalid memory management pointer
static const int FS_INVALIDENV =	0xFFFFFFF6;	///< Selected an invalid environment
static const int FS_ILLEGALFMT =	0xFFFFFFF5;	///< The exeucted file is in an invalid format
static const int FS_ILLEGALMOD =	0xFFFFFFF4;	///< Invalid open access mode
static const int FS_INVALIDPATH =	0xFFFFFFF3;	///< Mistake in selected file name
static const int FS_INVALIDPRM =	0xFFFFFFF2;	///< Called with an invalid parameter
static const int FS_INVALIDDRV =	0xFFFFFFF1;	///< Mistake in selected drive
static const int FS_DELCURDIR =		0xFFFFFFF0;	///< Unable to delete the current directory
static const int FS_NOTIOCTRL =		0xFFFFFFEF;	///< Unable to use IOCTRL with the device
static const int FS_LASTFILE =		0xFFFFFFEE;	///< Can not find any more files
static const int FS_CANTWRITE =		0xFFFFFFED;	///< Selected file can not be written
static const int FS_DIRALREADY =	0xFFFFFFEC;	///< Selected directory is already registered
static const int FS_CANTDELETE =	0xFFFFFFEB;	///< Can not delete because of a file
static const int FS_CANTRENAME =	0xFFFFFFEA;	///< Can not rename because of a file
static const int FS_DISKFULL =		0xFFFFFFE9;	///< Can not create a file because the disk is full
static const int FS_DIRFULL =		0xFFFFFFE8;	///< Can not create a file because the directory is full
static const int FS_CANTSEEK =		0xFFFFFFE7;	///< Can not seek in the selected location
static const int FS_SUPERVISOR =	0xFFFFFFE6;	///< Selected supervisor in supervisor mode
static const int FS_THREADNAME =	0xFFFFFFE5;	///< A thread with this name already exists
static const int FS_BUFWRITE =		0xFFFFFFE4;	///< Writing to inter-process communication buffers is disallowed
static const int FS_BACKGROUND =	0xFFFFFFE3;	///< Unable to start a background process
static const int FS_OUTOFLOCK =		0xFFFFFFE0;	///< Insufficient lock space
static const int FS_LOCKED =		0xFFFFFFDF;	///< Can not access because it is locked
static const int FS_DRIVEOPENED =	0xFFFFFFDE;	///< Selected drive has an open handler
static const int FS_LINKOVER =		0xFFFFFFDD;	///< The symbolic link is nested over 16 times
static const int FS_FILEEXIST =		0xFFFFFFB0;	///< The file exists

static const int FS_FATAL_MEDIAOFFLINE =	0xFFFFFFA3;	///< No media inserted
static const int FS_FATAL_WRITEPROTECT =	0xFFFFFFA2;	///< Write protected
static const int FS_FATAL_INVALIDCOMMAND =	0xFFFFFFA1;	///< Invalid command number
static const int FS_FATAL_INVALIDUNIT =	0xFFFFFFA0;	///< Invalid unit number

static const int HUMAN68K_PATH_MAX = 96;		///< Longest path allowed in Human68k

//===========================================================================
//
/// Human68k name space
//
//===========================================================================
namespace Human68k {
	/// File attribute bit
	enum attribute_t {
		AT_READONLY = 0x01,		///< Read only attribute
		AT_HIDDEN = 0x02,		///< Hidden attribute
		AT_SYSTEM = 0x04,		///< System attribute
		AT_VOLUME = 0x08,		///< Volume label attribute
		AT_DIRECTORY = 0x10,		///< Directory attribute
		AT_ARCHIVE = 0x20,		///< Archive attribute
		AT_ALL = 0xFF,			///< All attribute bits are 1
	};

	/// File open modes
	enum open_t {
		OP_READ = 0,			///< Read
		OP_WRITE = 1,			///< Write
		OP_FULL = 2,			///< Read/Write
		OP_MASK = 0x0F,			///< Decision mask
		OP_SHARE_NONE = 0x10,		///< Sharing forbidden
		OP_SHARE_READ = 0x20,		///< Read sharing
		OP_SHARE_WRITE = 0x30,		///< Write sharing
		OP_SHARE_FULL = 0x40,		///< Read/Write sharing
		OP_SHARE_MASK = 0x70,		///< Sharing decision mask
		OP_SPECIAL = 0x100,		///< Dictionary access
	};

	/// Seek types
	enum class seek_t {
		SK_BEGIN = 0,			///< From the beginning of a file
		SK_CURRENT = 1,			///< From the current location
		SK_END = 2,			///< From the end of the file
	};

	// Media byte
	const static int MEDIA_REMOTE = 0xF3;		///< Remote drive

	struct namests_t {
		uint8_t wildcard;			///< Wildcard character length
		uint8_t drive;			///< Drive number
		uint8_t path[65];			///< Path (subdirectory +/)
		uint8_t name[8];			///< File name (PADDING 0x20)
		uint8_t ext[3];			///< Extension (PADDING 0x20)
		uint8_t add[10];			///< File name addition (PADDING 0x00)

		void GetCopyPath(uint8_t* szPath) const;
		void GetCopyFilename(uint8_t* szFilename) const;
	};

	struct files_t {
		uint8_t fatr;			///< + 0 search attribute; read-only
		//		uint8_t drive;	///< + 1 drive number; read-only
		uint32_t sector;			///< + 2 directory sector; DOS _FILES first address substitute
		//		uint16_t cluster;	///< + 6 directory cluster; details unknown (unused)
		uint16_t offset;			///< + 8 directory entry; write-only
		//		uint8_t name[8];	///< +10 working file name; write-only (unused)
		//		uint8_t ext[3];	///< +18 working extension; write-only (unused)
		uint8_t attr;			///< +21 file attribute; write-only
		uint16_t time;			///< +22 last change time of day; write-only
		uint16_t date;			///< +24 last change date; write-only
		uint32_t size;			///< +26 file size; write-only
		uint8_t full[23];			///< +30 full name; write-only
	};

	struct fcb_t {
		//		uint8_t pad00[6];	///< + 0~+ 5	(unused)
		uint32_t fileptr;			///< + 6~+ 9	file pointer
		//		uint8_t pad01[4];	///< +10~+13	(unused)
		uint16_t mode;			///< +14~+15	open mode
		//		uint8_t pad02[16];	///< +16~+31	(unused)
		//		uint32_t zero;	///< +32~+35	zeros are written when opened (unused)
		//		uint8_t name[8];	///< +36~+43	file name (PADDING 0x20) (unused)
		//		uint8_t ext[3];	///< +44~+46	extension (PADDING 0x20) (unused)
		uint8_t attr;			///< +47	file attribute
		//		uint8_t add[10];	///< +48~+57	file name addition (PADDING 0x00) (unused)
		uint16_t time;			///< +58~+59	last change time of day
		uint16_t date;			///< +60~+61	last change date
		//		uint16_t cluster;	///< +62~+63	cluster number (unused)
		uint32_t size;			///< +64~+67	file size
		//		uint8_t pad03[28];	///< +68~+95	FAT cache (unused)
	};

	struct capacity_t {
		uint16_t freearea;			///< + 0	Number of available clusters
		uint16_t clusters;			///< + 2	Total number of clusters
		uint16_t sectors;			///< + 4	Number of sectors per cluster
		uint16_t bytes;			///< + 6	Number of bytes per sector
	};

	struct ctrldrive_t {
		uint8_t status;			///< +13	status
		uint8_t pad[3];			///< Padding
	};

	struct dpb_t {
		uint16_t sector_size;		///< + 0	Number of bytes in one sector
		uint8_t cluster_size;		///< + 2	Number sectors in one cluster -1
		uint8_t shift;			///< + 3	Number of cluster→sector shifts
		uint16_t fat_sector;		///< + 4	FAT first sector number
		uint8_t fat_max;			///< + 6	FAT storage quantity
		uint8_t fat_size;			///< + 7	FAT controlled sector number (excluding duplicates)
		uint16_t file_max;			///< + 8	Number of files in the root directory
		uint16_t data_sector;		///< +10	First sector number of data storage
		uint16_t cluster_max;		///< +12	Total number of clusters +1
		uint16_t root_sector;		///< +14	First sector number of root directory
		//	uint32_t driverentry;	///< +16	Device driver pointer (unused)
		uint8_t media;			///< +20	Media identifier
		//	uint8_t flag;		///< +21	Flag used by DPB (unused)
	};

	/// Directory entry struct
	struct dirent_t {
		uint8_t name[8];			///< + 0	File name (PADDING 0x20)
		uint8_t ext[3];			///< + 8	Extension (PADDING 0x20)
		uint8_t attr;			///< +11	File attribute
		uint8_t add[10];			///< +12	File name addition (PADDING 0x00)
		uint16_t time;			///< +22	Last change time of day
		uint16_t date;			///< +24	Last change date
		uint16_t cluster;			///< +26	Cluster number
		uint32_t size;			///< +28	File size
	};

	/// IOCTRL parameter union
	union ioctrl_t {
		uint8_t buffer[8];			///< Access in byte units
		uint32_t param;			///< Parameter (First 4 bytes)
		uint16_t media;			///< Media byte (First 2 bytes)
	};

	/// Command line parameter struct
	/**
	The driver itself is included in the beginning of the argument,
	so setting to a length longer than HUMAN68K_PATH_MAX
	*/
	struct argument_t {
		uint8_t buf[256];			///< Command line argument
	};
}

/// Number of FILES buffers
/**
Under normal circumstances it's enough with just a few buffers,
but Human68k multitasking may lead to multiple threads working
deeply in the system, which is why this value is set this high.

Default is 20 buffers.
*/
static const int XM6_HOST_FILES_MAX = 20;

/// Number of FCB buffers
/**
This decides how many files can be opened at the same time.

Default is 100 files.
*/
static const int XM6_HOST_FCB_MAX = 100;

/// Max number of virtual clusters and sectors
/**
Number of virtual sectors used for accessing the first sector of a file entity.
Allocating a generous amount to exceed the number of threads lzdsys uses for access.

Default is 10 sectors.
*/
static const int XM6_HOST_PSEUDO_CLUSTER_MAX = 10;

/// Number of caches for directory entries
/**
Human68k carries out a large number of checks of directory entries when doing an operation
inside a subdirectory. This specifies the number of caches used to speed up this operation.
Cache is allocated per drive. The more you add the faster it gets, but use too many
and the host OS gets under a heavy load, so be careful.

Default is 16.
*/
static const int XM6_HOST_DIRENTRY_CACHE_MAX = 16;

/// Max number of entries that can be stored per directory
/**
When a large number of files are stored in a directory, a larger amount of data than
contemporanous applications can handle will be returned. This may lead to errors such as
partial data being recognized, performance dropping significantly, or OOM crashes.
To guard against this, an upper limit is defined here. In the case of a particular
file manager, the upper limit is 2560 files. This is one good example to use as reference.

Default is around 60000 entries. (Upper limit of the FAT root directory)
*/
static const int XM6_HOST_DIRENTRY_FILE_MAX	= 65535;

/// Max number of patterns for file name deduplication
/**
The file names on the Human68k side are automatically created based on the file system on
the host side. However, Human68k have stricter file name length restrictions than the host has.
Because of this, there is a risk that file name duplication will occur. When this happens,
WindrvXM will use a certain renaming heuristic to generate alternate file names to resolve
the duplication. Theoretically, there are over 60 million (36^5) unique file names that
can be generated by this method. However, in reality any more than a few hundred
deduplications will take excessive processing time. So here an upper limit to deduplication
is set in order to maintain system performance. If a system is operated with common sense,
you should only need a few dozen deduplication patterns, so this value can be kept low
to further improve performance. In the case deduplication is not carried out, multiple files
with the same name will be created. When trying to access such files,
only the first entry will ever be accessed.

Default is 36 patterns.
*/
static const int XM6_HOST_FILENAME_PATTERN_MAX = 36;

/// Duplicate file identification mark
/**
A symbol used to distinguish between host and Human68k files.
Do not use a command shell escape character, or similar protected symbol.

Default is '@'.
*/
static const char XM6_HOST_FILENAME_MARK = '@';

/// WINDRV operational flags
/**
Normally set to 0. When put in the OS trash can for deletion, it is set to 1.
Other values are reserved for future use.
Can be used for future extentions such as internal operational flags or mock media byte.
*/
enum {
	WINDRV_OPT_REMOVE =		0x00000001,	///< Bit 0: File delete process			0:Directly 1:Trash can
	WINDRV_OPT_ALPHABET =		0x00000020,	///< Bit 5: File name comparison; Alphabet distinction		0:No 1:Yes	0:-C 1:+C
	WINDRV_OPT_COMPARE_LENGTH =	0x00000040,	///< Bit 6: File name comparison; String length (unimplemented)	0:18+3 1:8+3	0:+T 1:-T
	WINDRV_OPT_CONVERT_LENGTH =	0x00000080,	///< Bit 7: File name conversion; String length		0:18+3 1:8+3	0:-A 1:+A
	WINDRV_OPT_CONVERT_SPACE =	0x00000100,	///< Bit 8: File name conversion; Space		0:No 1:'_'
	WINDRV_OPT_CONVERT_BADCHAR =	0x00000200,	///< Bit 9: File name conversion; Invalid char		0:No 1:'_'
	WINDRV_OPT_CONVERT_HYPHENS =	0x00000400,	///< Bit10: File name conversion; Middle hyphen	0:No 1:'_'
	WINDRV_OPT_CONVERT_HYPHEN =	0x00000800,	///< Bit11: File name conversion; Initial hyphen	0:No 1:'_'
	WINDRV_OPT_CONVERT_PERIODS =	0x00001000,	///< Bit12: File name conversion; Middle period	0:No 1:'_'
	WINDRV_OPT_CONVERT_PERIOD =	0x00002000,	///< Bit13: File name conversion; Initial period	0:No 1:'_'
	WINDRV_OPT_REDUCED_SPACE =	0x00010000,	///< Bit16: File name reduction; Space		0:No 1:Reduced
	WINDRV_OPT_REDUCED_BADCHAR =	0x00020000,	///< Bit17: File name reduction; Invalid char		0:No 1:Reduced
	WINDRV_OPT_REDUCED_HYPHENS =	0x00040000,	///< Bit18: File name reduction Middle hyphen	0:No 1:Reduced
	WINDRV_OPT_REDUCED_HYPHEN =	0x00080000,	///< Bit19: File name reduction Initial hyphen	0:No 1:Reduced
	WINDRV_OPT_REDUCED_PERIODS =	0x00100000,	///< Bit20: File name reduction Middle period	0:No 1:Reduced
	WINDRV_OPT_REDUCED_PERIOD =	0x00200000,	///< Bit21: File name reduction Initial period	0:No 1:Reduced
	// Bit24～30 Duplicate file identification mark			0:Automatic 1～127:Chars
};

/// File system operational flag
/**
Normal is 0. Becomes 1 if attempting to mount in read-only mode.
Reserving the other values for future use.
Insurance against hard-to-detect devices such as homemade USB storage.
*/
static const uint32_t FSFLAG_WRITE_PROTECT = 0x00000001;	///< Bit0: Force write protect

//===========================================================================
//
/// Full ring list
///
/// First (root.next) is the most recent object.
/// Last (root.prev) is the oldest / unused object.
/// For code optimization purposes, always upcast the pointer when deleting.
//
//===========================================================================
class CRing {
public:
	CRing() { Init(); }
	~CRing() { Remove(); }
	CRing(CRing&) = default;
	CRing& operator=(const CRing&) = default;

	void  Init() { next = prev = this; }

	CRing*  Next() const { return next; }			///< Get the next element
	CRing*  Prev() const { return prev; }			///< Get the previous element

	void  Insert(CRing* pRoot)
	{
			// Separate the relevant objects
		assert(next);
		assert(prev);
		next->prev = prev;
		prev->next = next;
		// Insert into the beginning of the ring
		assert(pRoot);
		assert(pRoot->next);
		next = pRoot->next;
		prev = pRoot;
		pRoot->next->prev = this;
		pRoot->next = this;
	}
										///< Separate objects & insert into the beginning of the ring

	void  InsertTail(CRing* pRoot)
	{
			// Separate the relevant objects
		assert(next);
		assert(prev);
		next->prev = prev;
		prev->next = next;
		// Insert into the end of the ring
		assert(pRoot);
		assert(pRoot->prev);
		next = pRoot;
		prev = pRoot->prev;
		pRoot->prev->next = this;
		pRoot->prev = this;
	}
										///< Separate objects & insert into the end of the ring

	void  InsertRing(CRing* pRoot)
	{
			if (next == prev) return;

		// Insert into the beginning of the ring
		assert(pRoot);
		assert(pRoot->next);
		pRoot->next->prev = prev;
		prev->next = pRoot->next;
		pRoot->next = next;
		next->prev = pRoot;

		// Empty self
		next = prev = this;
	}
										///< Separate objects except self & insert into the beginning of the ring

	void  Remove()
	{
			// Separate the relevant objects
		assert(next);
		assert(prev);
		next->prev = prev;
		prev->next = next;
		// To be safe, assign self (nothing stops you from separating any number of times)
		next = prev = this;
	}
										///< Separate objects

private:
	CRing* next;						///< Next element
	CRing* prev;						///< Previous element
};

//===========================================================================
//
/// Directory Entry: File Name
//
//===========================================================================
class CHostFilename {
public:
	CHostFilename() = default;
	~CHostFilename() = default;

	static size_t Offset() { return offsetof(CHostFilename, m_szHost); }	///< Get offset location

	void SetHost(const TCHAR* szHost);					///< Set the name of the host
	const TCHAR* GetHost() const { return m_szHost; }	///< Get the name of the host
	void ConvertHuman(int nCount = -1);					///< Convert the Human68k name
	void CopyHuman(const uint8_t* szHuman);					///< Copy the Human68k name
	bool isReduce() const;							///< Inspect if the Human68k name is generated
	bool isCorrect() const { return m_bCorrect; }		///< Inspect if the Human68k file name adhers to naming rules
	const uint8_t* GetHuman() const { return m_szHuman; }	///< Get Human68k file name
	const uint8_t* GetHumanLast() const
	{ return m_pszHumanLast; }				///< Get Human68k file name
	const uint8_t* GetHumanExt() const { return m_pszHumanExt; }///< Get Human68k file name
	void SetEntryName();							///< Set Human68k directory entry
	void SetEntryAttribute(uint8_t nHumanAttribute)
	{ m_dirHuman.attr = nHumanAttribute; }			///< Set Human68k directory entry
	void  SetEntrySize(uint32_t nHumanSize)
	{ m_dirHuman.size = nHumanSize; }				///< Set Human68k directory entry
	void SetEntryDate(uint16_t nHumanDate)
	{ m_dirHuman.date = nHumanDate; }				///< Set Human68k directory entry
	void SetEntryTime(uint16_t nHumanTime)
	{ m_dirHuman.time = nHumanTime; }				///< Set Human68k directory entry
	void SetEntryCluster(uint16_t nHumanCluster)
	{ m_dirHuman.cluster = nHumanCluster; }			///< Set Human68k directory entry
	const Human68k::dirent_t*  GetEntry() const
	{ return &m_dirHuman; }					///< Get Human68k directory entry
	int CheckAttribute(uint32_t nHumanAttribute) const;			///< Determine Human68k directory entry attributes
	bool isSameEntry(const Human68k::dirent_t* pdirHuman) const
	{ assert(pdirHuman); return memcmp(&m_dirHuman, pdirHuman, sizeof(m_dirHuman)) == 0; }
										///< Determine Human68k directory entry match

	// Path name operations
	static const uint8_t* SeparateExt(const uint8_t* szHuman);			///< Extract extension from Human68k file name

private:
	static uint8_t* CopyName(uint8_t* pWrite, const uint8_t* pFirst, const uint8_t* pLast);
										///< Copy Human68k file name elements

	const uint8_t* m_pszHumanLast = nullptr;		///< Last position of the Human68k internal name of the relevant entry
	const uint8_t* m_pszHumanExt = nullptr;		///< Position of the extension of the Human68k internal name of the relevant entry
	bool m_bCorrect = false;			///< TRUE if the relevant entry of the Human68k internal name is correct
	uint8_t m_szHuman[24];			///< Human68k internal name of the relevant entry
	Human68k::dirent_t m_dirHuman;		///< All information for the Human68k relevant entry
	TCHAR m_szHost[FILEPATH_MAX];		///< The host name of the relevant entry (variable length)
};

//===========================================================================
//
/// Directory entry: path name
///
/// A file path in Human68k always begings with / and ends with /
/// They don't hold unit numbers.
/// Include the base path part of the name on the host side for a performance boost.
//
//===========================================================================
/** @note
Most Human68k applications are written in a way that expects time stamps not to
get updated for new directories created as a result of file operations, which
triggers updates to directory entires.
However, on the host file system, new directories do typically get an updated time stamp.

The unfortunate outcome is that when copying a directory for instance, the time stamp
will get overwritten even if the application did not intend for the time stamp to get updated.

Here follows an implementation of a directory cache FAT time stamp  emulation feature.
At the time of a file system update on the host side, time stamp information will be restored
in order to achieve expected behavior on the Human68k side.
*/
class CHostPath: public CRing {
	/// For memory management
	struct ring_t {
		CRing r;
		CHostFilename f;
	};

public:
	/// Search buffer
	struct find_t {
		uint32_t count;			///< Search execution count + 1 (When 0 the below value is invalid)
		uint32_t id;			///< Entry unique ID for the path of the next search
		const ring_t* pos;		///< Position of the next search (When identical to unique ID)
		Human68k::dirent_t entry;	///< Contents of the next seach entry

		void  Clear() { count = 0; }	///< Initialize
	};

	CHostPath() = default;
	~CHostPath();
	CHostPath(CHostPath&) = default;
	CHostPath& operator=(const CHostPath&) = default;

	void Clean();								///< Initialialize for reuse

	void SetHuman(const uint8_t* szHuman);					///< Directly specify the name on the Human68k side
	void SetHost(const TCHAR* szHost);					///< Directly specify the name on the host side
	bool isSameHuman(const uint8_t* szHuman) const;				///< Compare the name on the Human68k side
	bool isSameChild(const uint8_t* szHuman) const;				///< Compare the name on the Human68k side
	const TCHAR* GetHost() const { return m_szHost; }	///< Obtain the name on the host side
	const CHostFilename* FindFilename(const uint8_t* szHuman, uint32_t nHumanAttribute = Human68k::AT_ALL) const;
										///< Find file name
	const CHostFilename* FindFilenameWildcard(const uint8_t* szHuman, uint32_t nHumanAttribute, find_t* pFind) const;
										///< Find file name (with support for wildcards)
	bool isRefresh() const;							///< Check that the file change has been done
	void Refresh();							///< Refresh file
	void Backup();								/// Backup the time stamp on the host side
	void Restore() const;							/// Restore the time stamp on the host side
	void Release();							///< Update

	// CHostEntry is an external API that we use
	static void InitId() { g_nId = 0; }					///< Initialize the counter for the unique ID generation

private:
	static ring_t* Alloc(size_t nLength);					///< Allocate memory for the file name
	static void Free(ring_t* pRing);					///< Release memory for the file name
	static int Compare(const uint8_t* pFirst, const uint8_t* pLast, const uint8_t* pBufFirst, const uint8_t* pBufLast);
										///< Compare string (with support for wildcards)

	CRing m_cRing;								///< For CHostFilename linking
	time_t m_tBackup = 0;					///< For time stamp restoration
	bool m_bRefresh = true;						///< Refresh flag
	uint32_t m_nId = 0;								///< Unique ID (When the value has changed, it means an update has been made)
	uint8_t m_szHuman[HUMAN68K_PATH_MAX];					///< The internal Human68k name for the relevant entry
	TCHAR m_szHost[FILEPATH_MAX];						///< The host side name for the relevant entry

	static uint32_t g_nId;							///< Counter for the unique ID generation
};

//===========================================================================
//
/// File search processing
///
/// It's pretty much impossible to process Human68k file names as Unicode internally.
/// So, we carry out binary conversion for processing. We leave it up to the
/// directory entry cache to handle the conversion, which allows WINDRV to read
/// everything as Shift-JIS. Additionally, it allows Human68k names to be
/// fully independent of base path assignments.
///
/// We create directory entry cache just before file handling.
/// Since creating directory entires is very costly, we try to reuse created entries
/// as much as humanly possible.
///
/// There are three kinds of file search. They are all processed in CHostFiles::Find()
/// 1. Search by path name only; the only attribute is 'directory'; _CHKDIR _CREATE
/// 2. Path + file name + attribute search; _OPEN
/// 3. Path + wildcard + attribute search; _FILES _NFILES
/// The search results are kept as directory entry data.
//
//===========================================================================
class CHostFiles {
public:
	CHostFiles() = default;
	~CHostFiles() = default;

	void Init();

	void SetKey(uint32_t nKey) { m_nKey = nKey; }			///< Set search key
	bool isSameKey(uint32_t nKey) const { return m_nKey == nKey; }	///< Compare search key
	void SetPath(const Human68k::namests_t* pNamests);				///< Create path and file name internally
	bool isRootPath() const { return m_szHumanPath[1] == '\0'; }			///< Check if root directory
	void SetPathWildcard() { m_nHumanWildcard = 1; }				///< Enable file search using wildcards
	void SetPathOnly() { m_nHumanWildcard = 0xFF; }				///< Enable only path names
	bool isPathOnly() const { return m_nHumanWildcard == 0xFF; }			///< Check if set to only path names
	void SetAttribute(uint32_t nHumanAttribute) { m_nHumanAttribute = nHumanAttribute; }	///< Set search attribute
	bool Find(uint32_t nUnit, const class CHostEntry* pEntry);				///< Find files on the Human68k side, generating data on the host side
	const CHostFilename* Find(const CHostPath* pPath);					///< Find file name
	void SetEntry(const CHostFilename* pFilename);					///< Store search results on the Human68k side
	void SetResult(const TCHAR* szPath);						///< Set names on the host side
	void AddResult(const TCHAR* szPath);						///< Add file name to the name on the host side
	void AddFilename();								///< Add the new Human68k file name to the name on the host side

	const TCHAR* GetPath() const { return m_szHostResult; }		///< Get the name on the host side

	const Human68k::dirent_t* GetEntry() const { return &m_dirHuman; }///< Get Human68k directory entry

	uint32_t GetAttribute() const { return m_dirHuman.attr; }		///< Get Human68k attribute
	uint16_t GetDate() const { return m_dirHuman.date; }			///< Get Human68k date
	uint16_t GetTime() const { return m_dirHuman.time; }			///< Get Human68k time
	uint32_t GetSize() const { return m_dirHuman.size; }		///< Get Human68k file size
	const uint8_t* GetHumanFilename() const { return m_szHumanFilename; }///< Get Human68k file name
	const uint8_t* GetHumanResult() const { return m_szHumanResult; }	///< Get Human68k file name search results
	const uint8_t* GetHumanPath() const { return m_szHumanPath; }	///< Get Human68k path name

private:
	uint32_t m_nKey = 0;						///< FILES buffer address for Human68k; 0 is unused
	uint32_t m_nHumanWildcard = 0;				///< Human68k wildcard data
	uint32_t m_nHumanAttribute = 0;			///< Human68k search attribute
	CHostPath::find_t m_findNext = {};		///< Next search location data
	Human68k::dirent_t m_dirHuman = {};		///< Search results: Human68k file data
	uint8_t m_szHumanFilename[24] = {};		///< Human68k file name
	uint8_t m_szHumanResult[24] = {};			///< Search results: Human68k file name
	uint8_t m_szHumanPath[HUMAN68K_PATH_MAX] = {};	///< Human68k path name
	TCHAR m_szHostResult[FILEPATH_MAX] = {};	///< Search results: host's full path name
};

//===========================================================================
//
/// File search memory manager
//
//===========================================================================
class CHostFilesManager {
public:
#ifdef _DEBUG
	~CHostFilesManager();
#endif	// _DEBUG
	void  Init();						///< Initialization (when the driver is installed)
	void  Clean();						///< Release (when starting up or resetting)

	CHostFiles*  Alloc(uint32_t nKey);
	CHostFiles*  Search(uint32_t nKey);
	void  Free(CHostFiles* pFiles);
private:
	/// For memory management
	struct ring_t {
		CRing r;
		CHostFiles f;
	};

	CRing m_cRing;						///< For attaching to CHostFiles
};

//===========================================================================
//
/// FCB processing
//
//===========================================================================
class CHostFcb {
public:
	CHostFcb() = default;
	~CHostFcb() { Close(); }
	CHostFcb(CHostFcb&) = default;
	CHostFcb& operator=(const CHostFcb&) = default;

	void Init();

	void SetKey(uint32_t nKey) { m_nKey = nKey; }			///< Set search key
	bool isSameKey(uint32_t nKey) const { return m_nKey == nKey; }	///< Compare search key
	void SetUpdate() { m_bUpdate = true; }				///< Update
	bool isUpdate() const { return m_bUpdate; }			///< Get update state
	bool SetMode(uint32_t nHumanMode);						///< Set file open mode
	void SetFilename(const TCHAR* szFilename);					///< Set file name
	void SetHumanPath(const uint8_t* szHumanPath);					///< Set Human68k path name
	const uint8_t* GetHumanPath() const { return m_szHumanPath; }	///< Get Human68k path name

	bool Create(uint32_t nHumanAttribute, bool bForce);	///< Create file
	bool Open();									///< Open file
	uint32_t Read(uint8_t* pBuffer, uint32_t nSize);					///< Read file
	uint32_t Write(const uint8_t* pBuffer, uint32_t nSize);					///< Write file
	bool Truncate() const;								///< Truncate file
	uint32_t Seek(uint32_t nOffset, Human68k::seek_t nHumanSeek);		///< Seek file
	bool TimeStamp(uint32_t nHumanTime) const;						///< Set file time stamp
	void Close();									///< Close file

private:
	uint32_t m_nKey = 0;								///< Human68k FCB buffer address (0 if unused)
	bool m_bUpdate = false;							///< Update flag
	FILE* m_pFile = nullptr;						///< Host side file object
	const char* m_pszMode = nullptr;				///< Host side file open mode
	bool m_bFlag = false;							///< Host side file open flag
	uint8_t m_szHumanPath[HUMAN68K_PATH_MAX] = {};		///< Human68k path name
	TCHAR m_szFilename[FILEPATH_MAX] = {};			///< Host side file name
};

//===========================================================================
//
/// FCB processing manager
//
//===========================================================================
class CHostFcbManager {
public:
	#ifdef _DEBUG
	~CHostFcbManager();
	#endif	// _DEBUG
	void  Init();								///< Initialization (when the driver is installed)
	void  Clean() const;						///< Release (when starting up or resetting)

	CHostFcb*  Alloc(uint32_t nKey);
	CHostFcb*  Search(uint32_t nKey);
	void  Free(CHostFcb* p);

private:
	/// For memory management
	struct ring_t {
		CRing r;
		CHostFcb f;
	};

	CRing m_cRing;								///< For attaching to CHostFcb
};

//===========================================================================
//
/// Host side drive
///
/// Keeps the required data for each drive, managed in CHostEntry.
//
//===========================================================================
class CHostDrv
{
public:
	CHostDrv() = default;
	~CHostDrv();
	CHostDrv(CHostDrv&) = default;
	CHostDrv& operator=(const CHostDrv&) = default;

	void Init(const TCHAR* szBase, uint32_t nFlag);				///< Initialization (device startup and load)

	bool isWriteProtect() const { return m_bWriteProtect; }
	bool isEnable() const { return m_bEnable; }		///< Is it accessible?
	bool isMediaOffline() const;
	uint8_t GetMediaByte() const;
	uint32_t GetStatus() const;
	void SetEnable(bool);						///< Set media status
	bool CheckMedia();							///< Check if media was changed
	void Update();								///< Update media status
	void Eject();
	void GetVolume(TCHAR* szLabel);					///< Get volume label
	bool GetVolumeCache(TCHAR* szLabel) const;				///< Get volume label from cache
	uint32_t GetCapacity(Human68k::capacity_t* pCapacity);
	bool GetCapacityCache(Human68k::capacity_t* pCapacity) const;		///< Get capacity from cache

	// Cache operations
	void CleanCache() const;							///< Update all cache
	void CleanCache(const uint8_t* szHumanPath);				///< Update cache for the specified path
	void CleanCacheChild(const uint8_t* szHumanPath) const;	///< Update all cache below the specified path
	void DeleteCache(const uint8_t* szHumanPath);				///< Delete the cache for the specified path
	CHostPath* FindCache(const uint8_t* szHuman);				///< Inspect if the specified path is cached
	CHostPath* CopyCache(CHostFiles* pFiles);				///< Acquire the host side name on the basis of cache information
	CHostPath* MakeCache(CHostFiles* pFiles);				///< Get all required data to construct a host side name
	bool Find(CHostFiles* pFiles);						///< Find host side name (path + file name (can be abbreviated) + attribute)

private:
	// Path name operations
	static const uint8_t*  SeparateCopyFilename(const uint8_t* szHuman, uint8_t* szBuffer);
										///< Split and copy the first element of the Human68k full path name

	/// For memory management
	struct ring_t {
		CRing r;
		CHostPath f;
	};

	bool m_bWriteProtect = false;						///< TRUE if write-protected
	bool m_bEnable = false;							///< TRUE if media is usable
	uint32_t m_nRing = 0;							///< Number of stored path names
	CRing m_cRing;							///< For attaching to CHostPath
	Human68k::capacity_t m_capCache;				///< Sector data cache: if "sectors == 0" then not cached
	bool m_bVolumeCache = false;						///< TRUE if the volume label has been read
	TCHAR m_szVolumeCache[24] = {};					///< Volume label cache
	TCHAR m_szBase[FILEPATH_MAX] = {};					///< Base path
};

//===========================================================================
//
/// Directory entry management
//
//===========================================================================
class CHostEntry {

public:

	/// Max number of drive candidates
	static const int DRIVE_MAX = 10;

	CHostEntry() = default;
	~CHostEntry();
	CHostEntry(CHostEntry&) = default;
	CHostEntry& operator=(const CHostEntry&) = default;

	void Init() const;							///< Initialization (when the driver is installed)
	void Clean();								///< Release (when starting up or resetting)

	// Cache operations
	void CleanCache() const;							///< Update all cache
	void CleanCache(uint32_t nUnit) const;						///< Update cache for the specified unit
	void CleanCache(uint32_t nUnit, const uint8_t* szHumanPath) const;	///< Update cache for the specified path
	void CleanCacheChild(uint32_t nUnit, const uint8_t* szHumanPath) const;	///< Update cache below the specified path
	void DeleteCache(uint32_t nUnit, const uint8_t* szHumanPath) const;	///< Delete cache for the specified path
	bool Find(uint32_t nUnit, CHostFiles* pFiles) const;				///< Find host side name (path + file name (can be abbreviated) + attribute)
	void ShellNotify(uint32_t nEvent, const TCHAR* szPath);			///< Notify status change in the host side file system

	// Drive object operations
	void SetDrv(uint32_t nUnit, CHostDrv* pDrv);
	bool isWriteProtect(uint32_t nUnit) const;
	bool isEnable(uint32_t nUnit) const;					///< Is it accessible?
	bool isMediaOffline(uint32_t nUnit) const;
	uint8_t GetMediaByte(uint32_t nUnit) const;
	uint32_t GetStatus(uint32_t nUnit) const;					///< Get drive status
	bool CheckMedia(uint32_t nUnit) const;						///< Media change check
	void Eject(uint32_t nUnit) const;
	void GetVolume(uint32_t nUnit, TCHAR* szLabel) const;				///< Get volume label
	bool GetVolumeCache(uint32_t nUnit, TCHAR* szLabel) const;		///< Get volume label from cache
	uint32_t GetCapacity(uint32_t nUnit, Human68k::capacity_t* pCapacity) const;
	bool GetCapacityCache(uint32_t nUnit, Human68k::capacity_t* pCapacity) const;		///< Get cluster size from cache

private:

	CHostDrv* m_pDrv[DRIVE_MAX] = {};				///< Host side drive object
	uint32_t m_nTimeout = 0;							///< Last time a timeout check was carried out
};

//===========================================================================
//
/// Host side file system
//
//===========================================================================
/** @note
Current state of affairs:

While it violates the design philosophy of XM6, we should find a way for
'class Windrv' and 'class CWindrv' to have a direct pointer to 'class CFileSys'.
This way, we get the following benefits.

Benefit no. 1
Makes it possible to manage a large number of command handler methods in one place.
There is a high chance the command handlers will change drastically because of
host system architectural changes, so we will save a huge amount of maintenance work
in the long run.

Benefit no. 2
We would get rid of virtual funcion code for processing table creation and lookup.
It is not feasible to implement code in XM6 for simultaneous use of file system objects.
Therefore file system object polymorphism is a waste of CPU cycles.

I made the change as an experiment. Performance did improve.
The improvement was obvious from looking at the source the compiler spit out
after changing the FILESYS_FAST_STRUCTURE value in windrv.h.
You may understand now why I decided to rant here.

The easy solution is to put the content of 'class CFileSys' into 'class CWindrv'.
(To be honest, I really want to deprecate 'class CHost'... I wonder if there's a good way...)
*/
class CFileSys
{
public:
	CFileSys() = default;
	virtual ~CFileSys() = default;

	void Reset();								///< Reset (close all)
	void Init();								///< Initialization (device startup and load)

	// Command handlers
	uint32_t InitDevice(const Human68k::argument_t* pArgument);		///< $40 - Device startup
	int CheckDir(uint32_t nUnit, const Human68k::namests_t* pNamests) const;		///< $41 - Directory check
	int MakeDir(uint32_t nUnit, const Human68k::namests_t* pNamests) const;		///< $42 - Create directory
	int RemoveDir(uint32_t nUnit, const Human68k::namests_t* pNamests) const;	///< $43 - Delete directory
	int Rename(uint32_t nUnit, const Human68k::namests_t* pNamests, const Human68k::namests_t* pNamestsNew) const;
										///< $44 - Change file name
	int Delete(uint32_t nUnit, const Human68k::namests_t* pNamests) const;		///< $45 - Delete file
	int Attribute(uint32_t nUnit, const Human68k::namests_t* pNamests, uint32_t nHumanAttribute) const;
										///< $46 - Get / set file attribute
	int Files(uint32_t nUnit, uint32_t nKey, const Human68k::namests_t* pNamests, Human68k::files_t* pFiles);
										///< $47 - Find file
	int NFiles(uint32_t nUnit, uint32_t nKey, Human68k::files_t* pFiles);		///< $48 - Find next file
	int Create(uint32_t nUnit, uint32_t nKey, const Human68k::namests_t* pNamests, Human68k::fcb_t* pFcb, uint32_t nHumanAttribute, bool bForce);
										///< $49 - Create file
	int Open(uint32_t nUnit, uint32_t nKey, const Human68k::namests_t* pNamests, Human68k::fcb_t* pFcb);
										///< $4A - Open file
	int Close(uint32_t nUnit, uint32_t nKey, const Human68k::fcb_t* pFcb);		///< $4B - Close file
	int Read(uint32_t nKey, Human68k::fcb_t* pFcb, uint8_t* pAddress, uint32_t nSize);
										///< $4C - Read file
	int Write(uint32_t nKey, Human68k::fcb_t* pFcb, const uint8_t* pAddress, uint32_t nSize);
										///< $4D - Write file
	int Seek(uint32_t nKey, Human68k::fcb_t* pFcb, uint32_t nSeek, int nOffset);	///< $4E - Seek file
	uint32_t TimeStamp(uint32_t nUnit, uint32_t nKey, Human68k::fcb_t* pFcb, uint32_t nHumanTime);
										///< $4F - Get / set file timestamp
	int GetCapacity(uint32_t nUnit, Human68k::capacity_t* pCapacity) const;		///< $50 - Get capacity
	int CtrlDrive(uint32_t nUnit, Human68k::ctrldrive_t* pCtrlDrive) const;		///< $51 - Inspect / control drive status
	int GetDPB(uint32_t nUnit, Human68k::dpb_t* pDpb) const;				///< $52 - Get DPB
	int DiskRead(uint32_t nUnit, uint8_t* pBuffer, uint32_t nSector, uint32_t nSize);	///< $53 - Read sectors
	int DiskWrite(uint32_t nUnit) const;						///< $54 - Write sectors
	int Ioctrl(uint32_t nUnit, uint32_t nFunction, Human68k::ioctrl_t* pIoctrl);	///< $55 - IOCTRL
	int Flush(uint32_t nUnit) const;							///< $56 - Flush
	int CheckMedia(uint32_t nUnit) const;						///< $57 - Media change check
	int Lock(uint32_t nUnit) const;							///< $58 - Lock

	void SetOption(uint32_t nOption);						///< Set option
	uint32_t GetOption() const { return m_nOption; }		///< Get option
	uint32_t GetDefault() const { return m_nOptionDefault; }	///< Get default options
	static uint32_t GetFileOption() { return g_nOption; }			///< Get file name change option

	static const int DriveMax = CHostEntry::DRIVE_MAX;			///< Max number of drive candidates

private:
	void InitOption(const Human68k::argument_t* pArgument);
	bool FilesVolume(uint32_t nUnit, Human68k::files_t* pFiles) const;		///< Get volume label

	uint32_t m_nUnits = 0;								///< Number of current drive objects (Changes for every resume)

	uint32_t m_nOption = 0;							///< Current runtime flag
	uint32_t m_nOptionDefault = 0;						///< Runtime flag at reset

	uint32_t m_nDrives = 0;							///< Number of candidates for base path status restoration (scan every time if 0)

	uint32_t m_nKernel = 0;							///< Counter for kernel check
	uint32_t m_nKernelSearch = 0;						///< Initial address for NUL device

	uint32_t m_nHostSectorCount = 0;					///< Virtual sector identifier

	CHostFilesManager m_cFiles;						///< File search memory
	CHostFcbManager m_cFcb;							///< FCB operation memory
	CHostEntry m_cEntry;							///< Drive object and directory entry

	uint32_t m_nHostSectorBuffer[XM6_HOST_PSEUDO_CLUSTER_MAX];
										///< Entity that the virtual sector points to

	uint32_t m_nFlag[DriveMax] = {};					///< Candidate runtime flag for base path restoration
	TCHAR m_szBase[DriveMax][FILEPATH_MAX] = {};	///< Candidate for base path restoration
	static uint32_t g_nOption;							///< File name change flag
};
