/*
  Copyright (c) 2023 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem ;


#define _PATH_PROC_SWAPS        "/proc/swaps"
#define _PATH_PROC_MOUNTS       "/proc/mounts"
#define _PATH_PROC_DISKSTATS    "/proc/diskstats"
#define _PATH_PROC_STAT         "/proc/stat"
#define _PATH_PROC_VMSTAT       "/proc/vmstat"

#define _PATH_DEV_BYLABEL       "/dev/disk/by-label"
#define _PATH_DEV_BYUUID        "/dev/disk/by-uuid"

#define _PATH_PROC_CPUINFO      "/proc/cpuinfo"
#define _PATH_PROC_VERSION      "/proc/version"

#define _PATH_PROC_NETDEV       "/proc/net/dev"
#define _PATH_PROC_NETTCP       "/proc/net/tcp"
#define _PATH_PROC_NETTCP6      "/proc/net/tcp6"

#define _PATH_SYS_PCIDEVS       "/sys/bus/pci/devices"


enum SU_PARMS
{
	SU_DISKS,
	SU_JOURNAL,
	SU_MOUNTS,
	SU_NETWORK,
	SU_PCI,
	SU_SYSTEMD,
	SU_SYSINFO,
	SU_SYSUSE,
	SU_TASKS,
	SU_USB
} ;


enum CL_STATUS
{
	CL_RUNNING,
	CL_STARTING,
	CL_STOPPING,
	CL_STOPPED
} ;


class ut_format

{
	public:
	       int ut_min ;
	       int ut_max ;
} ;


#ifdef WITH_NETHOGS
class Update
{
	public:
		Update( int a,
			NethogsMonitorRecord r )
		{
			action = a ;
			record = r ;
		}

		Update()
		{
			action = 0 ;
		}

		int action ;
		NethogsMonitorRecord record ;
} ;
#endif


map<string, SU_PARMS> utils = { { "DISKS",   SU_DISKS   },
				{ "JOURNAL", SU_JOURNAL },
				{ "MOUNTS",  SU_MOUNTS  },
				{ "NET",     SU_NETWORK },
				{ "PCI",     SU_PCI     },
				{ "SYSTEMD", SU_SYSTEMD },
				{ "SYSINFO", SU_SYSINFO },
				{ "SYSUSE",  SU_SYSUSE  },
				{ "TASKS",   SU_TASKS   },
				{ "USB",     SU_USB     } } ;


map<SU_PARMS, ut_format> utFormat = { { SU_DISKS,   { 1, 1 } },
				      { SU_JOURNAL, { 1, 1 } },
				      { SU_MOUNTS,  { 1, 1 } },
				      { SU_NETWORK, { 1, 1 } },
				      { SU_PCI,     { 1, 1 } },
				      { SU_SYSTEMD, { 1, 1 } },
				      { SU_SYSINFO, { 1, 1 } },
				      { SU_SYSUSE,  { 1, 1 } },
				      { SU_TASKS,   { 1, 1 } },
				      { SU_USB,     { 1, 1 } } } ;


class if_dtls
{
	public:
		if_dtls()
		{
			point2point = false ;
		}

		string ipaddr ;
		string macaddr ;
		string nettype ;
		string netstat ;
		string ipaddr6 ;
		string bcaddr ;
		string bcaddr6 ;
		string netmask ;
		string netmask6 ;
		string dstaddr ;
		string dstaddr6 ;
		bool   point2point ;
} ;


class cpu_dtls
{
	public:
		cpu_dtls( const string& s )
		{
			stringstream ss( s ) ;
			ss.ignore( 5 ) ;

			ss >> user ;
			ss >> nice ;
			ss >> system ;
			ss >> idle ;
			ss >> iowait ;
			ss >> irq ;
			ss >> softirq ;
			ss >> steal ;
			ss >> guest ;
			ss >> gnice ;

			xI    = idle + iowait ;
			nxI   = user + nice + system + irq + softirq + steal  ;
			total = xI + nxI ;
		}

		cpu_dtls()
		{
			user    = 0 ;
			nice    = 0 ;
			system  = 0 ;
			idle    = 0 ;
			iowait  = 0 ;
			irq     = 0 ;
			softirq = 0 ;
			steal   = 0 ;
			guest   = 0 ;
			gnice   = 0 ;
			total   = 0 ;
		}

		string get_cpu( long clktck )
		{
			return addCommas( to_string( ( ( total - idle ) / clktck ) ) ) ;
		}

		string get_idle( long clktck )
		{
			return addCommas( to_string( ( idle / clktck ) ) ) ;
		}

		string get_cpu_pcent()
		{
			return addCommas( to_string( ( ( total - xI )/(float)total ) * 100 + 0.05 ), 1 ) ;
		}

		string get_user_pcent()
		{
			return addCommas( to_string( user/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_nice_pcent()
		{
			return addCommas( to_string( nice/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_idle_pcent()
		{
			return addCommas( to_string( idle/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_system_pcent()
		{
			return addCommas( to_string( system/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_iowait_pcent()
		{
			return addCommas( to_string( iowait/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_irq_pcent()
		{
			return addCommas( to_string( irq/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_softirq_pcent()
		{
			return addCommas( to_string( softirq/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_steal_pcent()
		{
			return addCommas( to_string( steal/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_guest_pcent()
		{
			return addCommas( to_string( guest/(float)total * 100 + 0.05 ), 1 ) ;
		}

		string get_gnice_pcent()
		{
			return addCommas( to_string( gnice/(float)total * 100 + 0.05 ), 1 ) ;
		}

		cpu_dtls operator - ( const cpu_dtls& rhs )
		{
			cpu_dtls temp ;
			temp.user    = user - rhs.user ;
			temp.nice    = nice - rhs.nice ;
			temp.system  = system - rhs.system ;
			temp.idle    = idle - rhs.idle ;
			temp.iowait  = iowait - rhs.iowait ;
			temp.irq     = irq - rhs.irq  ;
			temp.softirq = softirq - rhs.softirq ;
			temp.steal   = steal - rhs.steal ;
			temp.guest   = guest - rhs.guest ;
			temp.gnice   = gnice - rhs.gnice ;
			temp.total   = total - rhs.total ;
			temp.xI      = xI - rhs.xI ;
			temp.nxI     = nxI - rhs.nxI ;
			return temp ;
		}

		size_t user ;
		size_t nice ;
		size_t system ;
		size_t idle ;
		size_t iowait ;
		size_t irq ;
		size_t softirq ;
		size_t steal ;
		size_t guest ;
		size_t gnice ;
		size_t total ;
		size_t xI ;
		size_t nxI ;
} ;


class Unit_cc
{
	public:
		Unit_cc()
		{
			id          = nullptr ;
			description = nullptr ;
			loadState   = nullptr ;
			activeState = nullptr ;
			subState    = nullptr ;
			unitPath    = nullptr ;
			state       = nullptr ;
		}

		const char* id ;
		const char* description ;
		const char* loadState ;
		const char* activeState ;
		const char* subState ;
		const char* unitPath ;
		const char* state ;
} ;


class Unit_str
{
	public:
		Unit_str( Unit_cc* u )
		{
			description = ( u->description ) ?: "" ;
			loadState   = ( u->loadState )   ?: "" ;
			activeState = ( u->activeState ) ?: "" ;
			subState    = ( u->subState )    ?: "" ;
			unitPath    = ( u->unitPath )    ?: "" ;
			state       = ( u->state )       ?: "" ;
			set_state( state ) ;
		}

		Unit_str() {}

		Unit_str( const Unit_str& ca )
		{
			description = ca.description ;
			loadState   = ca.loadState ;
			activeState = ca.activeState ;
			subState    = ca.subState ;
			unitPath    = ca.unitPath ;
			Path        = ca.Path ;
			state       = ca.state ;
			state_str   = ca.state_str ;
		}

		Unit_str operator = ( const Unit_str& rhs )
		{
			description = rhs.description ;
			loadState   = rhs.loadState ;
			activeState = rhs.activeState ;
			subState    = rhs.subState ;
			state       = rhs.state ;
			state_str   = rhs.state_str ;

			return *this ;
		}

		void set_state( const string& s )
		{
			if ( s != "" )
			{
				state     = s ;
				state_str = ( state == "enabled"   ) ? "[x]" :
					    ( state == "disabled"  ) ? "[ ]" :
					    ( state == "static"    ) ? "[s]" :
					    ( state == "bad"       ) ? "-b-" :
					    ( state == "masked"    ) ? "-m-" :
					    ( state == "alias"     ) ? "-a-" :
					    ( state == "indirect"  ) ? "-i-" :
					    ( state == "generated" ) ? "-g-" :
					    ( state == "transient" ) ? "-t-" : "" ;
			}
		}

		const string& get_state() { return state ; }

		string description ;
		string loadState ;
		string activeState ;
		string subState ;
		string unitPath ;
		string Path ;
		string state_str ;

	private:
		string state ;
} ;



class psysutl : public pApplication
{
	public:
		psysutl() ;
		void application() ;

	private:
		boost::condition cond_collector ;

		boost::thread* bThread ;

		boost::mutex mtx ;

		vector<string> pstat_details1 ;
		vector<string> pstat_details2 ;

		vector<string> pvmstat_details1 ;
		vector<string> pvmstat_details2 ;

		bool collector_a ;

#ifdef WITH_NETHOGS
		static void onNethogsUpdate( int,
					     NethogsMonitorRecord const* ) ;
		static map<int, Update> row_updates ;
#endif


		static bool nethogs_running ;

		static boost::mutex mtxg ;

		string stderror ;

		void showSystemInfo() ;

		void showSystemdInfo() ;

		void showSystemdInfo_listUnits( sd_bus*,
						map<string, Unit_str*>& ) ;

		void showSystemdInfo_listUnitFiles( sd_bus*,
						    map<string, Unit_str*>& ) ;

		void showSystemdInfo_listUnitsByPatterns( sd_bus*,
							  map<string, Unit_str*>&,
							  const string& ) ;

		void showSystemdInfo_listUnitFilesByPatterns( sd_bus*,
							      map<string, Unit_str*>&,
							      const string& ) ;

		void showSystemdInfo_update( sd_bus*,
					     map<string, Unit_str*>&,
					     map<string, std::chrono::steady_clock::time_point>& ) ;

		void showSystemdInfo_buildTable( const string&,
						 map<string, Unit_str*>&,
						 string = "",
						 string = "",
						 string = "",
						 const string& = "",
						 const string& = "",
						 const string& = "",
						 const string& = "" ) ;

		void applyUnitChange( sd_bus*,
				      const char*,
				      const char* ) ;

		int bus_parse_unit_info( sd_bus_message*,
					 Unit_cc* ) ;

		string get_state( string,
				  sd_bus* ) ;

		void showSystemdInfo_Sessions( sd_bus* ) ;
		void showSystemdInfo_SessionsBuildTable( const string&,
							 sd_bus* ) ;
		void showSystemdInfo_SessionsDetails( sd_bus*,
						      const string& ) ;

		void showSystemdInfo_SessionsAction( sd_bus*,
						     const string&,
						     const string& ) ;
		string get_session_path( sd_bus*,
					 const char* ) ;


		void get_session_info( sd_bus*,
				       const string&,
				       string& ) ;


		void showSystemdInfo_Timers( sd_bus*,
					     map<string, Unit_str*>& ) ;
		void showSystemdInfo_TimersBuildTable( const string&,
						       sd_bus*,
						       map<string, Unit_str*>& ) ;
		void showSystemdInfo_TimerDetails( sd_bus*,
						   const string&,
						   string&,
						   string&,
						   string&,
						   string&,
						   string&,
						   string& ) ;

		void showJournal() ;
		void showJournal_setFilter() ;
		string showJournal_prompt( const vector<string>&,
					   const string& = "",
					   int = 1 ) ;

		bool showJournal_buildTable( const string& ) ;

		void showSystemUsage() ;
		void showSystemUsage_cleanup() ;
		void showSystemUsage_build( const string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string&,
					    string& ) ;
		void showSystemUsage_stats( vector<string>& ) ;
		void showSystemUsage_vmstats( vector<string>& ) ;
		void showSystemUsage_collector() ;

		void showMounts() ;
		void showMounts_build( const string&,
				       const string&,
				       const string&,
				       const string&,
				       const string&,
				       const string&,
				       string&,
				       string&,
				       string&,
				       string&,
				       string& ) ;
		void showMounts_info( const string&,
				      const string& ) ;

		void showBasic_info( const string& ) ;

		void showDisks() ;
		void showDisks_build( const string&,
				      string&,
				      string&,
				      string&,
				      string&,
				      string& ) ;
		void showDisks_partitions( const string& ) ;
		void showDisks_partbuild( const string&,
					  const string&,
					  string&,
					  string&,
					  string&,
					  string&,
					  string&,
					  string&,
					  string&,
					  string& ) ;
		void showDisks_mount( const string& ) ;

		void showDisks_unmount( const string& ) ;

		void showTasks() ;
		void showTasks_buildTable( const string&,
						 string&,
						 string&,
						 string&,
						 string&,
						 string&,
						 string&,
						 string&,
						 const string& = "",
						 const string& = "" ) ;

		void showUSB() ;
		void showUSB_build( const string&,
				    string&,
				    string&,
				    string&,
				    string&,
				    string&,
				    string&,
				    string& ) ;

		void DEV_list_all_properties( const string& ) ;
		void DEV_list_all_properties_add( const string&,
						  const string&,
						  string&,
						  string&,
						  size_t,
						  bool ) ;

		void DEV_list_all_sysattrs( const string& ) ;
		void DEV_list_all_sysattrs_add( const string&,
						const string&,
						string&,
						string&,
						size_t,
						bool ) ;

		void showNetwork() ;
		void showNetwork_build( const string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string& ) ;

		void showNetwork_get_details( map<string, if_dtls>& ) ;

		void showNetwork_info( const string&,
				       const string& ) ;

		void showNetwork_usage( const string& = "" ) ;
		void showNetwork_usage_build( const string&,
					      const string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      string&,
					      uint64_t&,
					      uint64_t& ) ;
		void showNetwork_usage_collector() ;
		void showNetwork_usage_cleanup() ;

		void showNetwork_stats( const string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string&,
					string& ) ;

		void showPCI() ;

		void showPCI_build( const string&,
				    string&,
				    string&,
				    string&,
				    string&,
				    string&,
				    string&,
				    string&,
				    string& ) ;

		void showDEV_diskstats( const string& ) ;

		string get_device_info( const string&,
					const string& ) ;

		void get_mnt_uuid_label( const string&,
					 string&,
					 string&,
					 string& ) ;

		string get_space( const string&,
				  const string& ) ;

		string get_pcent_space_used( const string& ) ;

		string get_processor( string& ) ;

		void get_swaps( set<string>& ) ;

		string convert_time( uint ) ;

		string ip_to_string( int,
				     void* ) ;

		string get_shared_var( const string& ) ;

		string get_dialogue_var( const string& ) ;

		string add_str( const string&,
				const string& ) ;

		void get_property( struct udev_device*,
				   const char*,
				   string& ) ;

		void get_sysattr( struct udev_device*,
				  const char*,
				  string&,
				  const char* = nullptr ) ;

		string get_username( uint32_t ) ;

		string format_kbs( float ) ;

		string get_program_name( const string& ) ;

		void set_message( string&,
				  const string&,
				  const string& ) ;

		void display_error( const string&,
				    const string& ) ;

		void display_error( const string&,
				    int ) ;

		void display_error( const string&,
				    const string&,
				    int,
				    int ) ;

		string get_mount_error( int ) ;

		int get_user_type() ;

		string get_tempname() ;

		void execute_cmd( int&,
				  const string&,
				  vector<string>& ) ;

		CL_STATUS coll_status ;
} ;
