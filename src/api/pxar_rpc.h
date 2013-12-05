#pragma once

#include "rpc.h"
#include "USBInterface.h"

class CTestboard
{
	RPC_DEFS
	RPC_THREAD

	CUSB usb;

public:
	CRpcIo& GetIo() { return *rpc_io; }

	CTestboard() { 
	  RPC_INIT rpc_io = &usb;
	}
	~CTestboard() { RPC_EXIT }

	int test(int a) { return a + 5; }

	// === RPC ==============================================================

	// Don't change the following two entries
	RPC_EXPORT uint16_t GetRpcVersion();
	RPC_EXPORT int32_t  GetRpcCallId(string &callName);

	RPC_EXPORT void GetRpcTimestamp(stringR &ts);

	RPC_EXPORT int32_t GetRpcCallCount();
	RPC_EXPORT bool    GetRpcCallName(int32_t id, stringR &callName);

	// === DTB connection ====================================================

	bool FindDTB(string &usbId) {
	  string name;
	  vector<string> devList;
	  unsigned int nDev;
	  unsigned int nr;

	  try {
	    if (!EnumFirst(nDev)) throw int(1);
	    for (nr=0; nr<nDev; nr++)	{
	      if (!EnumNext(name)) continue;
	      if (name.size() < 4) continue;
	      if (name.compare(0, 4, "DTB_") == 0) devList.push_back(name);
	    }
	  }
	  catch (int e) {
	    switch (e) {
	    case 1: printf("Cannot access the USB driver\n"); return false;
	    default: return false;
	    }
	  }

	  if (devList.size() == 0) {
	    printf("No DTB connected.\n");
	    return false;
	  }

	  if (devList.size() == 1) {
	    usbId = devList[0];
	    return true;
	  }

	  // If more than 1 connected device list them
	  printf("\nConnected DTBs:\n");
	  for (nr=0; nr<devList.size(); nr++) {
	    printf("%2u: %s", nr, devList[nr].c_str());
	    if (Open(devList[nr], false)) {
	      try {
		unsigned int bid = GetBoardId();
		printf("  BID=%2u\n", bid);
	      }
	      catch (...) {
		printf("  Not identifiable\n");
	      }
	      Close();
	    }
	    else printf(" - in use\n");
	  }

	  printf("Please choose DTB (0-%u): ", (nDev-1));
	  char choice[8];
	  fgets(choice, 8, stdin);
	  sscanf (choice, "%d", &nr);
	  if (nr >= devList.size()) {
	    nr = 0;
	    printf("No DTB opened\n");
	    return false;
	  }

	  usbId = devList[nr];
	  return true;
	};

	inline bool Open(string &name, bool init=true) {
	  rpc_Clear();
	  if (!usb.Open(&(name[0]))) return false;
	  if (init) Init();
	  return true;
	};

	void Close() {
	  usb.Close();
	  rpc_Clear();
	};

	bool EnumFirst(unsigned int &nDevices) { return usb.EnumFirst(nDevices); };
	bool EnumNext(string &name) {
	  char s[64];
	  if (!usb.EnumNext(s)) return false;
	  name = s;
	  return true;
	};
	bool Enum(unsigned int pos, string &name) {
	  char s[64];
	  if (!usb.Enum(s, pos)) return false;
	  name = s;
	  return true;
	};

	void SetTimeout(unsigned int timeout) { usb.SetTimeout(timeout); }

	bool IsConnected() { return usb.Connected(); }
	const char * ConnectionError()
	{ return usb.GetErrorMsg(usb.GetLastError()); }

	void Flush() { rpc_io->Flush(); }
	void Clear() { rpc_io->Clear(); }


	// === DTB identification ================================================

	RPC_EXPORT void GetInfo(stringR &info);
	RPC_EXPORT uint16_t GetBoardId();
	RPC_EXPORT void GetHWVersion(stringR &version);
	RPC_EXPORT uint16_t GetFWVersion();
	RPC_EXPORT uint16_t GetSWVersion();
	RPC_EXPORT uint16_t GetUser1Version();

	// === DTB service ======================================================

	// --- upgrade
	RPC_EXPORT uint16_t UpgradeGetVersion();
	RPC_EXPORT uint8_t  UpgradeStart(uint16_t version);
	RPC_EXPORT uint8_t  UpgradeData(string &record);
	RPC_EXPORT uint8_t  UpgradeError();
	RPC_EXPORT void     UpgradeErrorMsg(stringR &msg);
	RPC_EXPORT void     UpgradeExec(uint16_t recordCount);


	// === DTB functions ====================================================

	RPC_EXPORT void Init();
	RPC_EXPORT void Welcome();
	RPC_EXPORT void SetLed(uint8_t x);


	// --- Clock, Timing ----------------------------------------------------
	RPC_EXPORT void cDelay(uint16_t clocks);
	RPC_EXPORT void uDelay(uint16_t us);


	// --- Signal Delay -----------------------------------------------------
	#define SIG_CLK 0
	#define SIG_CTR 1
	#define SIG_SDA 2
	#define SIG_TIN 3

	#define SIG_MODE_NORMAL  0
	#define SIG_MODE_LO      1
	#define SIG_MODE_HI      2

	RPC_EXPORT void Sig_SetMode(uint8_t signal, uint8_t mode);
	RPC_EXPORT void Sig_SetPRBS(uint8_t signal, uint8_t speed);
	RPC_EXPORT void Sig_SetDelay(uint8_t signal, uint16_t delay, int8_t duty = 0);
	RPC_EXPORT void Sig_SetLevel(uint8_t signal, uint8_t level);
	RPC_EXPORT void Sig_SetOffset(uint8_t offset);
	RPC_EXPORT void Sig_SetLVDS();
	RPC_EXPORT void Sig_SetLCDS();


	// --- digital signal probe ---------------------------------------------
	#define PROBE_OFF     0
	#define PROBE_CLK     1
	#define PROBE_SDA     2
	#define PROBE_PGTOK   3
	#define PROBE_PGTRG   4
	#define PROBE_PGCAL   5
	#define PROBE_PGRESR  6
	#define PROBE_PGREST  7
	#define PROBE_PGSYNC  8
	#define PROBE_CTR     9
	#define PROBE_CLKP   10
	#define PROBE_CLKG   11
	#define PROBE_CRC    12

	RPC_EXPORT void SignalProbeD1(uint8_t signal);
	RPC_EXPORT void SignalProbeD2(uint8_t signal);


	// --- analog signal probe ----------------------------------------------
	#define PROBEA_TIN     0
	#define PROBEA_SDATA1  1
	#define PROBEA_SDATA2  2
	#define PROBEA_CTR     3
	#define PROBEA_CLK     4
	#define PROBEA_SDA     5
	#define PROBEA_TOUT    6
	#define PROBEA_OFF     7

	#define GAIN_1   0
	#define GAIN_2   1
	#define GAIN_3   2
	#define GAIN_4   3

	RPC_EXPORT void SignalProbeA1(uint8_t signal);
	RPC_EXPORT void SignalProbeA2(uint8_t signal);
	RPC_EXPORT void SignalProbeADC(uint8_t signal, uint8_t gain = 0);


	// --- ROC/Module power VD/VA -------------------------------------------
	RPC_EXPORT void Pon();	// switch ROC power on
	RPC_EXPORT void Poff();	// switch ROC power off

	RPC_EXPORT void _SetVD(uint16_t mV);
	RPC_EXPORT void _SetVA(uint16_t mV);
	RPC_EXPORT void _SetID(uint16_t uA100);
	RPC_EXPORT void _SetIA(uint16_t uA100);

	RPC_EXPORT uint16_t _GetVD();
	RPC_EXPORT uint16_t _GetVA();
	RPC_EXPORT uint16_t _GetID();
	RPC_EXPORT uint16_t _GetIA();

	void SetVA(double V) { _SetVA(uint16_t(V*1000)); }  // set VA voltage
	void SetVD(double V) { _SetVD(uint16_t(V*1000)); }  // set VD voltage
	void SetIA(double A) { _SetIA(uint16_t(A*10000)); }  // set VA current limit
	void SetID(double A) { _SetID(uint16_t(A*10000)); }  // set VD current limit

	double GetVA() { return _GetVA()/1000.0; }   // get VA voltage in V
	double GetVD() { return _GetVD()/1000.0; }	 // get VD voltage in V
	double GetIA() { return _GetIA()/10000.0; }  // get VA current in A
	double GetID() { return _GetID()/10000.0; }  // get VD current in A

	RPC_EXPORT void HVon();
	RPC_EXPORT void HVoff();
	RPC_EXPORT void ResetOn();
	RPC_EXPORT void ResetOff();
	RPC_EXPORT uint8_t GetStatus();
	RPC_EXPORT void SetRocAddress(uint8_t addr);

	RPC_EXPORT bool GetPixelAddressInverted();
	RPC_EXPORT void SetPixelAddressInverted(bool status);


	// --- pulse pattern generator ------------------------------------------
	#define PG_TOK   0x0100
	#define PG_TRG   0x0200
	#define PG_CAL   0x0400
	#define PG_RESR  0x0800
	#define PG_REST  0x1000
	#define PG_SYNC  0x2000

	RPC_EXPORT void Pg_SetCmd(uint16_t addr, uint16_t cmd);
	RPC_EXPORT void Pg_Stop();
	RPC_EXPORT void Pg_Single();
	RPC_EXPORT void Pg_Trigger();
	RPC_EXPORT void Pg_Loop(uint16_t period);

	// --- data aquisition --------------------------------------------------
	RPC_EXPORT uint32_t Daq_Open(uint32_t buffersize = 10000000); // max # of samples
	RPC_EXPORT void Daq_Close();
	RPC_EXPORT void Daq_Start();
	RPC_EXPORT void Daq_Stop();
	RPC_EXPORT uint32_t Daq_GetSize();
	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data, uint16_t blocksize = 16384);
	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data, uint16_t blocksize, uint32_t &availsize);
	RPC_EXPORT void Daq_Select_ADC(uint16_t blocksize, uint8_t source, uint8_t start, uint8_t stop = 0);
	RPC_EXPORT void Daq_Select_Deser160(uint8_t shift);


	// --- ROC/module Communication -----------------------------------------
	// -- set the i2c address for the following commands
	RPC_EXPORT void roc_I2cAddr(uint8_t id);
	// -- sends "ClrCal" command to ROC
	RPC_EXPORT void roc_ClrCal();
	// -- sets a single (DAC) register
	RPC_EXPORT void roc_SetDAC(uint8_t reg, uint8_t value);

	// -- set pixel bits (count <= 60)
	//    M - - - 8 4 2 1
	RPC_EXPORT void roc_Pix(uint8_t col, uint8_t row, uint8_t value);

	// -- trimm a single pixel (count < =60)
	RPC_EXPORT void roc_Pix_Trim(uint8_t col, uint8_t row, uint8_t value);

	// -- mask a single pixel (count <= 60)
	RPC_EXPORT void roc_Pix_Mask(uint8_t col, uint8_t row);

	// -- set calibrate at specific column and row
	RPC_EXPORT void roc_Pix_Cal(uint8_t col, uint8_t row, bool sensor_cal = false);

	// -- enable/disable a double column
	RPC_EXPORT void roc_Col_Enable(uint8_t col, bool on);

	// -- mask all pixels of a column and the coresponding double column
	RPC_EXPORT void roc_Col_Mask(uint8_t col);

	// -- mask all pixels and columns of the chip
	RPC_EXPORT void roc_Chip_Mask();

	// == TBM functions =====================================================
	RPC_EXPORT bool TBM_Present(); 
	RPC_EXPORT void tbm_Enable(bool on);
	RPC_EXPORT void tbm_Addr(uint8_t hub, uint8_t port);
	RPC_EXPORT void mod_Addr(uint8_t hub);
	RPC_EXPORT void tbm_Set(uint8_t reg, uint8_t value);
	RPC_EXPORT bool tbm_Get(uint8_t reg, uint8_t &value);
	RPC_EXPORT bool tbm_GetRaw(uint8_t reg, uint32_t &value);

	// --- Wafer test functions
	RPC_EXPORT bool testColPixel(uint8_t col, uint8_t trimbit, vectorR<uint8_t> &res);

	// Ethernet test functions
	RPC_EXPORT void Ethernet_Send(string &message);
	RPC_EXPORT uint32_t Ethernet_RecvPackets();

	
	RPC_EXPORT int32_t CountReadouts(int32_t nTriggers);
	RPC_EXPORT int32_t CountReadouts(int32_t nTriggers, int32_t chipId);
	RPC_EXPORT int32_t CountReadouts(int32_t nTriggers, int32_t dacReg, int32_t dacValue);
	RPC_EXPORT int32_t PixelThreshold(int32_t col, int32_t row, int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim);
	RPC_EXPORT int32_t PH(int32_t col, int32_t row, int32_t trim, int16_t nTriggers);
	RPC_EXPORT bool test_pixel_address(int32_t col, int32_t row);

};