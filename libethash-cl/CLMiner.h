/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#pragma once

#include <libdevcore/Worker.h>
#include <libethcore/EthashAux.h>
#include <libethcore/Miner.h>
#include <libhwmon/wrapnvml.h>
#include <libhwmon/wrapadl.h>
#if defined(__linux)
#include <libhwmon/wrapamdsysfs.h>
#endif

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS true
#define CL_HPP_ENABLE_EXCEPTIONS true
#define CL_HPP_CL_1_2_DEFAULT_BUILD true
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#include "CL/cl2.hpp"

// macOS OpenCL fix:
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV       0x4000
#endif

#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV       0x4001
#endif

#define OPENCL_PLATFORM_UNKNOWN 0
#define OPENCL_PLATFORM_NVIDIA  1
#define OPENCL_PLATFORM_AMD     2
#define OPENCL_PLATFORM_CLOVER  3

static const unsigned int factor_table[] = {
	0x80000f01,0x1fc07f41,0xfc1000f9,0xfa233a51,0x1f07c7d1,0x3d9811b7,0x7a44df53,0xf2b9d815,0xf0f100e3,0x77978815,
	0xed73056f,0x3aef7591,0xea0ea5ef,0xe86603d7,0xe6c2cca9,0x7292db4f,0xe38e40cb,0xe1fc9be7,0xe07070fd,0x6f74b1f1,
	0xdd67d021,0xdbeb6369,0x6d3a0bed,0xd901f59,0xd79442a9,0xd62ba96f,0x3531e07b,0x34da03a5,0xd20d3fcb,0x342dab9b,
	0x67b2481d,0x670b49c5,0x66666c29,0x1970ea2f,0xca458927,0xc907fe13,0xc7ce0db5,0x634c107,0x62b2ed29,0xc43730b3,
	0xc30c3d89,0xc1e4c875,0xc0c0c8b1,0xbfa037c1,0xbe83063b,0x5eb488b,0x5e293b39,0x1767dd4b,0xba2e930b,0x5c90a49b,
	0xb817080d,0x5b87e453,0xb60b6bd7,0xb509e98b,0x5a05a0d9,0xb30f6837,0xb21645b,0x1623fad3,0x58160e81,0x579d713b,
	0xae4c4f45,0xad602fef,0xac76af79,0xab8f8b1f,0xaaaabb8f,0xa9c85233,0x2a3a1547,0xa80aa08b,0x14e5e31b,0x532ae289,
	0xa57ebadd,0x5254e8cd,0xa3d70cb3,0x518331f7,0x511be533,0xa16b3857,0x505054a5,0x9fd81f0b,0x27c461c7,0x279335cd,
	0x9d89e0f3,0x9cc8e221,0x9c09c8c9,0x9b4c71d5,0x26a43d6d,0x133ae4cf,0x991f31ef,0x9868c8bf,0x97b42f0f,0x97013019,
	0x964ff551,0x4ad01883,0x94f223b7,0x25116355,0x939a8e69,0x92f1215b,0x4924929d,0x91a2ce4f,0x487edefb,0x482d20f7,
	0x47dc156f,0x478bc37d,0x8e783749,0x46ed3011,0x8d3dccdd,0x8ca2a77d,0x8c08cab9,0x45b81cd1,0x456c8333,0x8a431887,
	0x89ae45bf,0x891ad347,0x11111321,0x87f7835b,0x8767b95d,0x86d90927,0x864b8c25,0x85bf3905,0x429a060f,0x425505c1,
	0x42108687,0x839932f7,0x83127aa7,0x828cc363,0x82082317,0x40c24c35,0x204086b9,0x20202285,0x80000181,0xff01,
	0x7f020857,0xfd091d01,0x1f81faab,0xfb188a35,0xfa232fd,0xf92fb8c3,0x7c1f1b09,0xf74e4a07,0xf6604b7d,0x7aba0261,
	0xf4899b1,0x3ce8431b,0x3cae780b,0x78ea4823,0x3c3c3c75,0xf00f1a71,0x1de5d887,0xee500fc3,0xed73238b,0x764bda0f,
	0x75dedc4b,0xeae56d47,0x7507579,0xe93974e5,0xe865d9cf,0x39e4eb3f,0xe6c2bed9,0xe5f38a8b,0xe525a43,0x39164ce9,
	0x71c71cd7,0x38b12a39,0xe1fc7d83,0xe135ada9,0xe0703f07,0xdfac24cd,0xdee96dbd,0x6f13fbfd,0x6eb3e7b1,0xdca8ff77,
	0xdbeb6a0d,0xdb2f221d,0x6d3a0731,0xd9ba45f5,0x36406d0b,0xd84a73f9,0x6bca2265,0x6b6fa30d,0xd62b8457,0xd578efbf,
	0xd4c79493,0xd41752cd,0x69b409ad,0x695d067d,0xd20d3c6d,0xd161624b,0xd0b6a473,0x68068aa5,0xcf6487b1,0xcebcfab1,
	0x19c2d2d9,0x335c4aa3,0xccccd51f,0xcc298eb,0xcb872d71,0x32b97873,0xca459005,0xc9a64877,0x3241f7f7,0x190d5011
};

static const unsigned int exponent_table[] = {
	22, 20, 23, 23, 20, 21, 22, 23, 23, 22,
	23, 21, 23, 23, 23, 22, 23, 23, 23, 22,
	23, 23, 22, 19, 23, 23, 21, 21, 23, 21,
	22, 22, 22, 20, 23, 23, 23, 18, 22, 23,
	23, 23, 23, 23, 23, 18, 22, 20, 23, 22,
	23, 22, 23, 23, 22, 23, 19, 20, 22, 22,
	23, 23, 23, 23, 23, 23, 21, 23, 20, 22,
	23, 22, 23, 22, 22, 23, 22, 23, 21, 21,
	23, 23, 23, 23, 21, 20, 23, 23, 23, 23,
	23, 22, 23, 21, 23, 23, 22, 23, 22, 22,
	22, 22, 23, 22, 23, 23, 23, 22, 22, 23,
	23, 23, 20, 23, 23, 23, 23, 23, 22, 22,
	22, 23, 23, 23, 23, 22, 21, 21, 23, 8,
	23, 24, 21, 24, 20, 24, 23, 24, 24, 23,
	20, 22, 22, 23, 22, 24, 21, 24, 24, 23,
	23, 24, 19, 24, 24, 22, 24, 24, 20, 22,
	23, 22, 24, 24, 24, 24, 24, 23, 23, 24,
	24, 24, 23, 24, 22, 24, 23, 23, 24, 24,
	24, 24, 23, 23, 24, 24, 24, 23, 24, 24,
	21, 22, 24, 20, 24, 22, 24, 24, 22, 21
};

static const unsigned char Keccak_rho_etc_LUT[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x82,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
	0x8a,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x80,0x00,0x80,
	0x00,0x00,0x00,0x80,0x8b,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x81,0x80,0x00,0x80,
	0x00,0x00,0x00,0x80,0x09,0x80,0x00,0x00,0x00,0x00,0x00,0x80,
	0x8a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x09,0x80,0x00,0x80,0x00,0x00,0x00,0x00,
	0x0a,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x8b,0x80,0x00,0x80,
	0x00,0x00,0x00,0x00,0x8b,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
	0x89,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x80,0x00,0x00,
	0x00,0x00,0x00,0x80,0x02,0x80,0x00,0x00,0x00,0x00,0x00,0x80,
	0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x0a,0x80,0x00,0x00,
	0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x80,0x00,0x00,0x00,0x80,
	0x81,0x80,0x00,0x80,0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,
	0x00,0x00,0x00,0x80,0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,
	0x08,0x80,0x00,0x80,0x00,0x00,0x00,0x80,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x82,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
	0x8a,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x80,0x00,0x80,
	0x00,0x00,0x00,0x80,0x8b,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x81,0x80,0x00,0x80,
	0x00,0x00,0x00,0x80,0x09,0x80,0x00,0x00,0x00,0x00,0x00,0x80,
	0x8a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x09,0x80,0x00,0x80,0x00,0x00,0x00,0x00,
	0x0a,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x8b,0x80,0x00,0x80,
	0x00,0x00,0x00,0x00,0x8b,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
	0x89,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x80,0x00,0x00,
	0x00,0x00,0x00,0x80,0x02,0x80,0x00,0x00,0x00,0x00,0x00,0x80,
	0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x0a,0x80,0x00,0x00,
	0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x80,0x00,0x00,0x00,0x80,
	0x81,0x80,0x00,0x80,0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,
	0x00,0x00,0x00,0x80,0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,
	0x08,0x80,0x00,0x80,0x00,0x00,0x00,0x80 
};

namespace dev
{
namespace eth
{

class CLMiner: public Miner
{
public:
	/* -- default values -- */
	/// Default value of the local work size. Also known as workgroup size.
	static const unsigned c_defaultLocalWorkSize = 128;
	/// Default value of the global work size as a multiplier of the local work size
	static const unsigned c_defaultGlobalWorkSizeMultiplier = 8192;

	CLMiner(FarmFace& _farm, unsigned _index);
	~CLMiner();

	static unsigned instances() { return s_numInstances > 0 ? s_numInstances : 1; }
	static unsigned getNumDevices();
	static void listDevices();
	static bool configureGPU(
		unsigned _localWorkSize,
		unsigned _globalWorkSizeMultiplier,
		unsigned _platformId,
		uint64_t _currentBlock,
		unsigned _dagLoadMode,
		unsigned _dagCreateDevice,
		unsigned _version,
		unsigned _ethIntensity
	);
	static void setNumInstances(unsigned _instances) { s_numInstances = std::min<unsigned>(_instances, getNumDevices()); }
	static void setThreadsPerHash(unsigned _threadsPerHash){s_threadsPerHash = _threadsPerHash; }
	static void setDevices(unsigned * _devices, unsigned _selectedDeviceCount)
	{
		for (unsigned i = 0; i < _selectedDeviceCount; i++)
		{
			s_devices[i] = _devices[i];
		}
	}
	HwMonitor hwmon() override;
protected:
	void kickOff() override;
	void pause() override;

private:
	void workLoop() override;
	void report(uint64_t _nonce, WorkPackage const& _w);
	void detectAmdGpu(string gpu_name, unsigned char **_buffer, unsigned int *_bufferSize, unsigned int version);

	bool init(const h256& seed);

	cl::Context m_context;
	cl::CommandQueue m_queue;

	bool m_useAsmKernel = false;
	unsigned m_asmKernelVer = 0;

	cl::Kernel m_searchKernel;
	cl::Kernel m_asmSearchKernel;

	cl::Kernel m_asmDagKernel;
	cl::Kernel m_dagKernel;
	
	cl::Buffer m_dag;
	cl::Buffer m_light;

	cl::Buffer m_header;
	cl::Buffer m_dualHeader;

	cl::Buffer m_searchBuffer;

	unsigned m_globalWorkSize = 0;
	unsigned m_workgroupSize = 0;
	unsigned m_dagSize = 0;

	static unsigned s_platformId;
	static unsigned s_numInstances;
	static unsigned s_threadsPerHash;
	static int s_devices[16];

	/// The local work size for the search
	static unsigned s_workgroupSize;
	/// The initial global work size for the searches
	static unsigned s_initialGlobalWorkSize;
	static unsigned s_asmVersion;
	static unsigned s_ethIntensity;


	wrap_nvml_handle *nvmlh = NULL;
	wrap_adl_handle *adlh = NULL;
#if defined(__linux)
	wrap_amdsysfs_handle *sysfsh = NULL;
#endif
};

}
}
