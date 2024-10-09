/*  Compile with ::                                                                                                      */
/* without nethogs                                                                                                       */
/* g++ -ggdb -shared -fPIC -std=c++11 -Wl,-soname,libpsysutl.so -o libpsysutl.so -ludev -lsystemd psysutl.cpp            */
/*                                                                                                                       */
/* with nethogs                                                                                                          */
/* g++ -ggdb -shared -fPIC -std=c++11 -Wl,-soname,libpsysutl.so -o libpsysutl.so -ludev -lnethogs -lsystemd psysutl.cpp  */

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


/****************************************************************************/
/* System utilities:                                                        */
/*                                                                          */
/* Basic system information.                                                */
/* CPU Usage.                                                               */
/* Task display.                                                            */
/* File system mounts.                                                      */
/* Display disks and partitions.                                            */
/* USB devices.                                                             */
/* Network.                                                                 */
/* PCI devices.                                                             */
/* Systemd units, timers and sessions.                                      */
/* Journal logs.                                                            */
/*                                                                          */
/****************************************************************************/

//#define WITH_NETHOGS

#include <functional>
#include <net/if.h>

#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/mount.h>

#include <libudev.h>

#include <mntent.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <pwd.h>

#include <systemd/sd-bus.h>
#include <boost/scope_exit.hpp>
#include <boost/regex.hpp>

#ifdef WITH_NETHOGS
#include "libnethogs.h"
#endif

#include <boost/thread/thread.hpp>

#include "../lspfall.h"
#include "psysutl.h"

using namespace boost ;
using namespace boost::filesystem ;

#ifdef WITH_NETHOGS
map<int, Update>psysutl::row_updates ;
#endif

boost::mutex psysutl::mtxg ;
bool psysutl::nethogs_running = false ;

LSPF_APP_MAKER( psysutl )


psysutl::psysutl()
{
	STANDARD_HEADER( "System utilities to show system information, mounts, disks, etc..", "1.0.0" )
}


void psysutl::application()
{
	int ws = words( PARM ) ;

	string w1 = upper( word( PARM, 1 ) ) ;

	auto it = utils.find( w1 ) ;
	if ( it == utils.end() )
	{
		setmsg( "PSUT011A" ) ;
		llog( "E", "Invalid parameter passed to PSYSUTL: " << PARM << endl ) ;
		uabend() ;
	}

	auto ut = utFormat.find( it->second ) ;
	if ( ut->second.ut_min != -1 && ws < ut->second.ut_min )
	{
		setmsg( "PSUT011B" ) ;
		return ;
	}

	if ( ut->second.ut_max != -1 && ws > ut->second.ut_max )
	{
		setmsg( "PSUT011C" ) ;
		return ;
	}

	switch ( it->second )
	{
	case SU_DISKS:
			showDisks() ;
			break ;
	case SU_JOURNAL:
			showJournal() ;
			break ;
	case SU_MOUNTS:
			showMounts() ;
			break ;
	case SU_NETWORK:
			showNetwork() ;
			break ;
	case SU_PCI:
			showPCI() ;
			break ;
	case SU_SYSTEMD:
			showSystemdInfo() ;
			break ;

	case SU_SYSINFO:
			showSystemInfo() ;
			break ;

	case SU_SYSUSE:
			showSystemUsage() ;
			break ;

	case SU_TASKS:
			showTasks() ;
			break ;

	case SU_USB:
			showUSB() ;
			break ;
	}
}


/**************************************************************************************************************/
/**********************************          SYSTEM INFORMATION             ***********************************/
/**************************************************************************************************************/

void psysutl::showSystemInfo()
{
	//
	// Show some basic system information.
	//

	double gigs = 1024 * 1024 * 1024 ;

	float factor = (float)( 1 << SI_LOAD_SHIFT ) ;

	string zcmd ;

	string iuptime ;
	string inprocs ;
	string itram ;
	string ifram ;
	string itswap ;
	string ifswap ;
	string imodel ;
	string ilvers ;
	string ivendor ;
	string iavload1 ;
	string iavload2 ;
	string iavload3 ;

	const string vlist1 = "ZCMD IUPTIME INPROCS ITRAM IFRAM IMODEL ILVERS IVENDOR IAVLOAD1 IAVLOAD2 IAVLOAD3" ;
	const string vlist2 = "ITSWAP IFSWAP" ;

	struct sysinfo info ;

	if ( sysinfo( &info ) == -1 )
	{
		display_error( "PSUT014A", strerror( errno ) ) ;
		return ;
	}

	vdefine( vlist1, &zcmd, &iuptime, &inprocs, &itram, &ifram, &imodel, &ilvers, &ivendor, &iavload1, &iavload2, &iavload3 ) ;
	vdefine( vlist2, &itswap, &ifswap ) ;

	imodel = get_processor( ivendor ) ;
	ilvers = get_shared_var( "ZOSREL" ) + " (" + get_shared_var( "ZMACHINE" ) + ")" ;

	while ( true )
	{
		iuptime  = convert_time( info.uptime ) ;
		inprocs  = addCommas( info.procs ) ;
		itram    = addCommas( to_string( info.totalram * info.mem_unit / gigs + 0.05 ), 1 ) ;
		ifram    = addCommas( to_string( info.freeram * info.mem_unit / gigs + 0.05 ), 1 ) ;
		itswap   = addCommas( to_string( info.totalswap * info.mem_unit / gigs + 0.05 ), 1 ) ;
		ifswap   = addCommas( to_string( info.freeswap * info.mem_unit / gigs + 0.05 ), 1 ) ;
		iavload1 = addCommas( to_string( info.loads[ 0 ] / factor + 0.005 ), 2 ) ;
		iavload2 = addCommas( to_string( info.loads[ 1 ] / factor + 0.005 ), 2 ) ;
		iavload3 = addCommas( to_string( info.loads[ 2 ] / factor + 0.005 ), 2 ) ;

		display( "PSUT00SI" ) ;
		if ( RC > 0) { break ; }

		sysinfo( &info ) ;
	}

	vdelete( vlist1, vlist2 ) ;
}


/**************************************************************************************************************/
/**********************************        SYSTEM USAGE INFORMATION         ***********************************/
/**************************************************************************************************************/

void psysutl::showSystemUsage()
{
	//
	// Show system usage.
	// Collector runs in a separate task to read procfs every 1s.
	//

	string zcmd ;
	string panel ;
	string tabName ;

	string pswpin ;
	string pswpout ;
	string pgin ;
	string pgout ;

	string cpu ;
	string user ;
	string nice ;
	string system ;
	string idle ;
	string iowait ;
	string irq ;
	string softirq ;
	string steal ;
	string guest ;
	string gnice ;

	string tcpu ;
	string tuser ;
	string tnice ;
	string tsystem ;
	string tidle ;
	string tiowait ;
	string tirq ;
	string tsoftirq ;
	string tsteal ;
	string tguest ;
	string tgnice ;

	int ztdtop  = 1 ;
	int ztdsels = 0 ;

	coll_status = CL_STOPPED ;
	collector_a = true ;

	control( "ABENDRTN", static_cast<void (pApplication::*)()>(&psysutl::showSystemUsage_cleanup) ) ;

	bThread = new boost::thread( &psysutl::showSystemUsage_collector, this ) ;

	while ( coll_status != CL_RUNNING )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 50 ) ) ;
	}

	string sel ;

	const string vlist1 = "ZTDTOP ZTDSELS" ;
	const string vlist2 = "CPU USER NICE SYSTEM IDLE IOWAIT IRQ SOFTIRQ STEAL GUEST GNICE" ;
	const string vlist3 = "TCPU TUSER TNICE TSYSTEM TIDLE TIOWAIT TIRQ TSOFTIRQ TSTEAL TGUEST TGNICE" ;
	const string vlist4 = "ZCMD SEL PSWPIN PSWPOUT PGIN PGOUT" ;

	vdefine( vlist1, &ztdtop, &ztdsels ) ;
	vdefine( vlist2, &cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &gnice ) ;
	vdefine( vlist3, &tcpu, &tuser, &tnice, &tsystem, &tidle, &tiowait, &tirq, &tsoftirq, &tsteal, &tguest, &tgnice ) ;
	vdefine( vlist4, &zcmd, &sel, &pswpin, &pswpout, &pgin, &pgout ) ;

	tabName = "USG" + d2ds( taskid(), 5 ) ;

	showSystemUsage_build( tabName,
			       pswpin,
			       pswpout,
			       pgin,
			       pgout,
			       cpu,
			       user,
			       nice,
			       system,
			       idle,
			       iowait,
			       irq,
			       softirq,
			       steal,
			       guest,
			       gnice,
			       tcpu,
			       tuser,
			       tnice,
			       tsystem,
			       tidle,
			       tiowait,
			       tirq,
			       tsoftirq,
			       tsteal,
			       tguest,
			       tgnice ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00W1" ;
		}
		tbdispl( tabName, panel ) ;
		if ( RC > 4 ) { break ; }
		panel = "" ;
		if ( ztdsels < 2 )
		{
			showSystemUsage_build( tabName,
					       pswpin,
					       pswpout,
					       pgin,
					       pgout,
					       cpu,
					       user,
					       nice,
					       system,
					       idle,
					       iowait,
					       irq,
					       softirq,
					       steal,
					       guest,
					       gnice,
					       tcpu,
					       tuser,
					       tnice,
					       tsystem,
					       tidle,
					       tiowait,
					       tirq,
					       tsoftirq,
					       tsteal,
					       tguest,
					       tgnice ) ;
		}
		zcmd = "" ;
	}

	coll_status = CL_STOPPING ;
	cond_collector.notify_all() ;
	while ( coll_status != CL_STOPPED )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	delete bThread ;

	tbend( tabName ) ;
	vdelete( vlist1, vlist2, vlist3, vlist4 ) ;
}


void psysutl::showSystemUsage_cleanup()
{
	//
	// Stop the background data collector when an abend in the main thread occurs
	// or we'll bring down the whole of lspf.
	//

	if ( coll_status != CL_STOPPED )
	{
		coll_status = CL_STOPPING ;
		cond_collector.notify_all() ;
		while ( coll_status != CL_STOPPED )
		{
			boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
		}
	}
}


void psysutl::showSystemUsage_build( const string& tabName,
				     string& pswpin,
				     string& pswpout,
				     string& pgin,
				     string& pgout,
				     string& cpu,
				     string& user,
				     string& nice,
				     string& system,
				     string& idle,
				     string& iowait,
				     string& irq,
				     string& softirq,
				     string& steal,
				     string& guest,
				     string& gnice,
				     string& tcpu,
				     string& tuser,
				     string& tnice,
				     string& tsystem,
				     string& tidle,
				     string& tiowait,
				     string& tirq,
				     string& tsoftirq,
				     string& tsteal,
				     string& tguest,
				     string& tgnice )
{
	//
	// Build the lspf table with the individual CPU usage stats.
	//

	long clktck = sysconf( _SC_CLK_TCK ) ;

	vector<cpu_dtls> cpu_details1 ;
	vector<cpu_dtls> cpu_details2 ;

	int pgpgin1  = 0 ;
	int pgpgin2  = 0 ;
	int pgpgout1 = 0 ;
	int pgpgout2 = 0 ;

	int pswpin1  = 0 ;
	int pswpin2  = 0 ;
	int pswpout1 = 0 ;
	int pswpout2 = 0 ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	for ( size_t i = 0 ; i < pstat_details1.size() ; ++i )
	{
		const string* t1 ;
		const string* t2 ;
		if ( collector_a )
		{
			t1 = &pstat_details1[ i ] ;
			t2 = &pstat_details2[ i ] ;
		}
		else
		{
			t2 = &pstat_details1[ i ] ;
			t1 = &pstat_details2[ i ] ;
		}
		if ( t1->compare( 0, 3, "cpu" ) == 0 )
		{
			cpu_details1.push_back( cpu_dtls( *t1 ) ) ;
			cpu_details2.push_back( cpu_dtls( *t2 ) ) ;
		}
	}

	for ( size_t i = 0 ; i < pvmstat_details1.size() ; ++i )
	{
		const string* t1 ;
		const string* t2 ;
		if ( collector_a )
		{
			t1 = &pvmstat_details1[ i ] ;
			t2 = &pvmstat_details2[ i ] ;
		}
		else
		{
			t2 = &pvmstat_details1[ i ] ;
			t1 = &pvmstat_details2[ i ] ;
		}
		if ( t1->compare( 0, 6, "pgpgin" ) == 0 )
		{
			pgpgin1 = ds2d( word( *t1, 2 ) ) ;
			pgpgin2 = ds2d( word( *t2, 2 ) ) ;
		}
		else if ( t1->compare( 0, 7, "pgpgout" ) == 0 )
		{
			pgpgout1 = ds2d( word( *t1, 2 ) ) ;
			pgpgout2 = ds2d( word( *t2, 2 ) ) ;
		}
		else if ( t1->compare( 0, 6, "pswpin" ) == 0 )
		{
			pswpin1 = ds2d( word( *t1, 2 ) ) ;
			pswpin2 = ds2d( word( *t2, 2 ) ) ;
		}
		else if ( t1->compare( 0, 7, "pswpout" ) == 0 )
		{
			pswpout1 = ds2d( word( *t1, 2 ) ) ;
			pswpout2 = ds2d( word( *t2, 2 ) ) ;
		}
	}

	pswpin  = to_string( pswpin2 - pswpin1 ) ;
	pswpout = to_string( pswpout2 - pswpout1 ) ;
	pgin    = to_string( pgpgin2 - pgpgin1 ) ;
	pgout   = to_string( pgpgout2 - pgpgout1 ) ;

	cpu_dtls& t2   = cpu_details2[ 0 ] ;
	cpu_dtls delta = cpu_details2[ 0 ] - cpu_details1[ 0 ] ;

	cpu     = t2.get_cpu( clktck ) ;
	idle    = t2.get_idle( clktck ) ;
	nice    = addCommas( to_string( t2.nice / clktck ) ) ;
	system  = addCommas( to_string( t2.system / clktck ) ) ;
	idle    = addCommas( to_string( t2.idle / clktck ) ) ;
	iowait  = addCommas( to_string( t2.iowait / clktck ) ) ;
	irq     = addCommas( to_string( t2.irq / clktck ) ) ;
	softirq = addCommas( to_string( t2.softirq / clktck ) ) ;
	steal   = addCommas( to_string( t2.steal / clktck ) ) ;
	guest   = addCommas( to_string( t2.guest / clktck ) ) ;
	gnice   = addCommas( to_string( t2.gnice / clktck ) ) ;

	tbcreate( tabName,
		  "",
		  "(TCPUNUM,TCPU,TUSER,TNICE,TSYSTEM,TIDLE,TIOWAIT,TIRQ,TSOFTIRQ,TSTEAL,TGUEST,TGNICE)",
		  NOWRITE,
		  REPLACE ) ;

	for ( size_t i = 0 ; i < cpu_details1.size() ; ++i )
	{
		cpu_dtls delta = cpu_details2[ i ] - cpu_details1[ i ] ;
		tcpu     = delta.get_cpu_pcent() ;
		tuser    = delta.get_user_pcent() ;
		tnice    = delta.get_nice_pcent() ;
		tsystem  = delta.get_system_pcent() ;
		tiowait  = delta.get_iowait_pcent() ;
		tirq     = delta.get_irq_pcent() ;
		tsoftirq = delta.get_softirq_pcent() ;
		tsteal   = delta.get_steal_pcent() ;
		tguest   = delta.get_guest_pcent() ;
		tgnice   = delta.get_gnice_pcent() ;
		tidle    = delta.get_idle_pcent() ;
		vreplace( "TCPUNUM", ( i == 0 ) ? "all" : "CPU" + to_string( i ) ) ;
		tbadd( tabName ) ;
	}
}


void psysutl::showSystemUsage_collector()
{
	//
	// This method runs in the background to read /proc/stat and /proc/vmstat data every 1s.
	//

	boost::mutex mutex ;

	int i = 9 ;

	showSystemUsage_stats( pstat_details1 ) ;
	showSystemUsage_vmstats( pvmstat_details1 ) ;

	boost::this_thread::sleep_for( boost::chrono::milliseconds( 150 ) ) ;

	showSystemUsage_stats( pstat_details2 ) ;
	showSystemUsage_vmstats( pvmstat_details2 ) ;

	coll_status = CL_RUNNING ;

	while ( coll_status == CL_RUNNING )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_collector.wait_for( lk, boost::chrono::milliseconds( 100 ) ) ;
		lk.unlock() ;
		++i ;
		if ( i == 1 )
		{
			boost::lock_guard<boost::mutex> lock( mtx ) ;
			if ( collector_a )
			{
				collector_a = false ;
				showSystemUsage_stats( pstat_details1 ) ;
				showSystemUsage_vmstats( pvmstat_details1 ) ;
			}
			else
			{
				collector_a = true ;
				showSystemUsage_stats( pstat_details2 ) ;
				showSystemUsage_vmstats( pvmstat_details2 ) ;
			}
		}
		else if ( i == 10 )
		{
			i = 0 ;
		}
	}

	coll_status = CL_STOPPED ;
}


void psysutl::showSystemUsage_stats( vector<string>& v )
{
	//
	// Get proc stats data.
	//

	string line ;

	v.clear() ;

	std::ifstream if_procstats( _PATH_PROC_STAT ) ;

	while ( getline( if_procstats, line ) )
	{
		v.push_back( line ) ;
	}

	if_procstats.close() ;
}


void psysutl::showSystemUsage_vmstats( vector<string>& v )
{
	//
	// Get proc vmstats data.
	//

	string line ;

	v.clear() ;

	std::ifstream if_pvmstats( _PATH_PROC_VMSTAT ) ;

	while ( getline( if_pvmstats, line ) )
	{
		v.push_back( line ) ;
	}

	if_pvmstats.close() ;
}


/**************************************************************************************************************/
/**********************************          FILE SYSTEM MOUNTS             ***********************************/
/**************************************************************************************************************/

void psysutl::showMounts()
{
	//
	// Show mounted file systems.
	//

	string zcmd ;
	string panel ;
	string tabName ;
	string oblkdev ;

	string w1 ;
	string w2 ;

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	string filter_fsname ;
	string filter_dir ;
	string filter_type ;
	string filter_opts ;

	string sel ;
	string cursor ;
	string fsname ;
	string dir ;
	string type ;
	string opts ;

	string listid ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD OBLKDEV SEL FSNAME DIR TYPE OPTS" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &oblkdev, &sel, &fsname, &dir, &type, &opts ) ;

	vget( "OBLKDEV", PROFILE ) ;

	tabName = "MNT" + d2ds( taskid(), 5 ) ;

	showMounts_build( tabName,
			  oblkdev,
			  filter_fsname,
			  filter_dir,
			  filter_type,
			  filter_opts,
			  sel,
			  fsname,
			  dir,
			  type,
			  opts ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00MT" ;
		}
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		w1 = word( zcmd, 1 ) ;
		w2 = word( zcmd, 2 ) ;
		if ( w1 == "NAME" )
		{
			filter_fsname = w2 ;
			ztdsels = 0 ;
		}
		else if ( w1 == "MOUNT" )
		{
			filter_dir = w2 ;
			ztdsels = 0 ;
		}
		else if ( w1 == "TYPE" )
		{
			filter_type = w2 ;
			ztdsels = 0 ;
		}
		else if ( w1 == "OPTS" )
		{
			filter_opts = w2 ;
			ztdsels = 0 ;
		}
		else if ( w1 == "RESET" )
		{
			filter_fsname = "" ;
			filter_dir    = "" ;
			filter_type   = "" ;
			filter_opts   = "" ;
			ztdsels = 0 ;
		}
		else if ( sel == "I" )
		{
			showMounts_info( dir, fsname ) ;
		}
		else if ( sel == "L" )
		{
			lmdinit( "LID", dir ) ;
			vcopy( "LID", listid ) ;
			lmddisp( listid ) ;
			lmdfree( listid ) ;
		}
		else if ( sel == "Q" )
		{
			showDEV_diskstats( fsname ) ;
		}
		else if ( sel == "U" )
		{
			showDisks_unmount( fsname ) ;
		}
		if ( ztdsels < 2 )
		{
			showMounts_build( tabName,
					  oblkdev,
					  filter_fsname,
					  filter_dir,
					  filter_type,
					  filter_opts,
					  sel,
					  fsname,
					  dir,
					  type,
					  opts ) ;

		}
		zcmd = "" ;
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showMounts_build( const string& tabName,
				const string& oblkdev,
				const string& filter_fsname,
				const string& filter_dir,
				const string& filter_type,
				const string& filter_opts,
				string& sel,
				string& fsname,
				string& dir,
				string& type,
				string& opts )
{
	//
	// Build the lspf mount table from getmntent.
	//

	struct mntent* ent ;

	FILE* afile ;

	tbcreate( tabName,
		  "",
		  "(SEL,FSNAME,DIR,TYPE,OPTS)",
		  NOWRITE,
		  REPLACE ) ;

	afile = setmntent( _PATH_PROC_MOUNTS, "r" ) ;

	if ( afile )
	{
		while ( ( ent = getmntent( afile ) ) )
		{
			sel    = "" ;
			fsname = ent->mnt_fsname ;
			if ( oblkdev == "/" && fsname.front() != '/' )
			{
				continue ;
			}
			if ( filter_fsname != "" && upper( fsname ).find( filter_fsname ) == string::npos )
			{
				continue ;
			}
			dir = ent->mnt_dir ;
			if ( filter_dir != "" && upper( dir ).find( filter_dir ) == string::npos )
			{
				continue ;
			}
			type = ent->mnt_type ;
			if ( filter_type != "" && upper( type ).find( filter_type ) == string::npos )
			{
				continue ;
			}
			opts = ent->mnt_opts ;
			if ( filter_opts != "" && upper( opts ).find( filter_opts ) == string::npos )
			{
				continue ;
			}
			tbadd( tabName ) ;
		}
	}

	tbsort( tabName, "(FSNAME,C,A)" ) ;
	tbtop( tabName ) ;

	endmntent( afile ) ;
}


void psysutl::showMounts_info( const string& dir,
			       const string& fsname )
{
	//
	// Display information for a particular mount.
	//

	string ientry  = dir ;
	string ifsname = fsname ;

	string mnt_type ;
	string mnt_opts ;

	string immode ;
	string imsync ;
	string imiguid ;
	string ibsize ;
	string ibfree ;
	string ibavail ;
	string iblocks ;
	string ipcentu ;
	string ifiles ;
	string iffree ;
	string ifavail ;
	string ifsid ;

	string imaj ;
	string imin ;
	string iuuid ;
	string ilabel ;

	struct stat results ;

	struct statvfs buf ;

	const string vlist1 = "IENTRY IFSNAME IMMODE IMSYNC IMIGUID IBSIZE IBFREE IBAVAIL IBLOCKS IPCENTU" ;
	const string vlist2 = "IFILES IFFREE IFAVAIL IFSID IMAJ IMIN IUUID ILABEL ITYPE IOPTS" ;

	vdefine( vlist1, &ientry, &ifsname, &immode, &imsync, &imiguid, &ibsize, &ibfree, &ibavail, &iblocks, &ipcentu ) ;
	vdefine( vlist2, &ifiles, &iffree, &ifavail, &ifsid, &imaj, &imin, &iuuid, &ilabel, &mnt_type, &mnt_opts ) ;

	struct mntent* ent ;

	FILE* afile = setmntent( _PATH_PROC_MOUNTS, "r" ) ;

	if ( afile )
	{
		while ( ( ent = getmntent( afile ) ) )
		{
			if ( string( ent->mnt_type ) != "autofs" && string( ent->mnt_dir ) == dir )
			{
				mnt_type = ent->mnt_type ;
				mnt_opts = ent->mnt_opts ;
				break ;
			}
		}
	}

	endmntent( afile ) ;

	if ( statvfs( dir.c_str(), &buf ) == -1 )
	{
		display_error( "PSUT012A", strerror( errno ) ) ;
	}
	else
	{
		ibsize  = addCommas( buf.f_bsize ) ;
		ibfree  = addCommas( buf.f_bfree ) ;
		ibavail = addCommas( buf.f_bavail ) ;
		iblocks = addCommas( buf.f_blocks ) ;
		ifiles  = addCommas( buf.f_files ) ;
		iffree  = addCommas( buf.f_ffree ) ;
		ifavail = addCommas( buf.f_favail ) ;
		ifsid   = ui2ds( buf.f_fsid ) ;
		immode  = ( buf.f_flag & ST_RDONLY ) ? "RO" : "R/W" ;
		imsync  = ( buf.f_flag & ST_SYNCHRONOUS ) ? "YES" : "NO" ;
		imiguid = ( buf.f_flag & ST_NOSUID ) ? "YES" : "NO" ;
		if ( buf.f_blocks > 0 )
		{
			ipcentu = to_string( ( ( ( buf.f_blocks - buf.f_bavail ) * 1000 / buf.f_blocks ) + 5 ) / 10 ) + "%" ;
		}
	}

	if ( lstat( dir.c_str(), &results ) == 0 )
	{
		imaj = to_string( major( results.st_dev ) ) ;
		imin = to_string( minor( results.st_dev ) ) ;
	}

	iuuid  = get_device_info( fsname, _PATH_DEV_BYUUID ) ;
	ilabel = get_device_info( fsname, _PATH_DEV_BYLABEL ) ;

	RC = 0 ;

	while ( RC == 0 )
	{
		display( "PSUT00MI" ) ;
	}

	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showBasic_info( const string& fsname )
{
	//
	// Display basic information for an unmounted partition.
	//

	string ifsname = fsname ;

	string ientry ;
	string immode ;
	string imsync ;
	string imiguid ;
	string ibsize ;
	string ibfree ;
	string ibavail ;
	string iblocks ;
	string ipcentu ;
	string ifiles ;
	string iffree ;
	string ifavail ;
	string ifsid ;

	string imaj ;
	string imin ;
	string iuuid ;
	string ilabel ;

	const string vlist1 = "IENTRY IFSNAME IMMODE IMSYNC IMIGUID IBSIZE IBFREE IBAVAIL IBLOCKS IPCENTU" ;
	const string vlist2 = "IFILES IFFREE IFAVAIL IFSID IMAJ IMIN IUUID ILABEL" ;

	vdefine( vlist1, &ientry, &ifsname, &immode, &imsync, &imiguid, &ibsize, &ibfree, &ibavail, &iblocks, &ipcentu ) ;
	vdefine( vlist2, &ifiles, &iffree, &ifavail, &ifsid, &imaj, &imin, &iuuid, &ilabel ) ;

	iuuid  = get_device_info( fsname, _PATH_DEV_BYUUID ) ;
	ilabel = get_device_info( fsname, _PATH_DEV_BYLABEL ) ;

	RC = 0 ;

	while ( RC == 0 )
	{
		display( "PSUT00MJ" ) ;
	}

	vdelete( vlist1, vlist2 ) ;
}


/**************************************************************************************************************/
/**********************************           BLOCK SYBSYSTEM               ***********************************/
/**************************************************************************************************************/

void psysutl::showDisks()
{
	//
	// List BLOCK devices.
	//

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	string sel ;
	string disk ;
	string model ;
	string size ;
	string syspath ;
	string tabName ;
	string cursor ;
	string panel ;
	string zcmd ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD SEL DISK MODEL SIZE SYSPATH" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &sel, &disk, &model, &size, &syspath ) ;

	tabName = "DSK" + d2ds( taskid(), 5 ) ;

	showDisks_build( tabName,
			 sel,
			 disk,
			 model,
			 size,
			 syspath ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00D1" ;
		}
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "P" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_properties( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "Q" )
		{
			showDEV_diskstats( disk ) ;
		}
		else if ( sel == "S" )
		{
			control( "DISPLAY", "SAVE" ) ;
			showDisks_partitions( disk ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "T" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_sysattrs( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		if ( ztdsels < 2 )
		{
			showDisks_build( tabName,
					 sel,
					 disk,
					 model,
					 size,
					 syspath ) ;
		}
		zcmd = "" ;
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showDisks_build( const string& tabName,
			       string& sel,
			       string& disk,
			       string& model,
			       string& size,
			       string& syspath )
{
	//
	// Build the lspf table with the disks.
	//

	struct udev* udev = udev_new() ;

	struct udev_device* dev ;
	struct udev_device* dev_parent ;

	struct udev_list_entry* devices ;
	struct udev_list_entry* entry ;

	const char* path ;
	const char* blks ;
	const char* blksize ;

	tbcreate( tabName,
		  "",
		  "(SEL,DISK,MODEL,SIZE,SYSPATH)",
		  NOWRITE,
		  REPLACE ) ;

	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	struct udev_enumerate* enumerate = udev_enumerate_new( udev ) ;

	udev_enumerate_add_match_subsystem( enumerate, "block" ) ;
	udev_enumerate_scan_devices( enumerate ) ;

	devices = udev_enumerate_get_list_entry( enumerate ) ;

	udev_list_entry_foreach( entry, devices )
	{
		path = udev_list_entry_get_name( entry ) ;
		dev  = udev_device_new_from_syspath( udev, path ) ;
		if ( dev )
		{
			if ( udev_device_get_devnode( dev ) && string( udev_device_get_devtype( dev ) ) == "disk" )
			{
				tbvclear( tabName ) ;
				syspath = path ;
				disk    = udev_device_get_devnode( dev ) ;
				dev_parent = udev_device_get_parent( dev ) ;
				if ( dev_parent )
				{
					get_sysattr( dev_parent, "model", model ) ;
				}
				blksize = udev_device_get_sysattr_value( dev, "queue/physical_block_size" ) ;
				if ( blksize )
				{
					blks = udev_device_get_sysattr_value( dev, "size" ) ;
					if ( blks )
					{
						size = get_space( blksize, blks ) ;
					}
				}
				tbadd( tabName ) ;
			}
			udev_device_unref( dev ) ;
		}
	}

	udev_enumerate_unref( enumerate ) ;

	udev_unref( udev ) ;

	tbsort( tabName, "(DISK,C,A)" ) ;
	tbtop( tabName ) ;
}


void psysutl::showDisks_partitions( const string& disk )
{
	//
	// List partitions for a disk.
	//

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;
	int ppos    = 0 ;

	int crp ;

	string sel ;
	string part ;
	string size ;
	string pcent ;
	string mount ;
	string uuid ;
	string label ;
	string syspath ;

	string tabName ;
	string cursor ;
	string panel ;

	string zcmd ;
	string zverb ;

	string listid ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD ZVERB SEL PART SIZE PCENT MOUNT UUID LABEL SYSPATH" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &zverb, &sel, &part, &size, &pcent, &mount, &uuid, &label, &syspath ) ;

	tabName = "PRT" + d2ds( taskid(), 5 ) ;

	showDisks_partbuild( tabName,
			     disk,
			     sel,
			     part,
			     size,
			     pcent,
			     mount,
			     uuid,
			     label,
			     syspath ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = ( ppos == 0 ) ? "PSUT00D2" : "PSUT00D3" ;
		}
		control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		control( "PASSTHRU", "LRSCROLL", "PASOFF" ) ;
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" && ppos > 0 )
		{
			--ppos ;
			continue ;
		}
		else if ( zverb == "RIGHT" && ppos < 1 )
		{
			++ppos ;
			continue ;
		}
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "I" )
		{
			( mount != "" ) ? showMounts_info( mount, part ) : showBasic_info( part ) ;
		}
		else if ( sel == "L" )
		{
			lmdinit( "LID", mount ) ;
			vcopy( "LID", listid ) ;
			lmddisp( listid ) ;
			lmdfree( listid ) ;
		}
		else if ( sel == "P" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_properties( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "M" )
		{
			showDisks_mount( part ) ;
		}
		else if ( sel == "Q" )
		{
			showDEV_diskstats( part ) ;
		}
		else if ( sel == "T" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_sysattrs( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "U" )
		{
			showDisks_unmount( part ) ;
		}
		if ( ztdsels < 2 )
		{
			showDisks_partbuild( tabName,
					     disk,
					     sel,
					     part,
					     size,
					     pcent,
					     mount,
					     uuid,
					     label,
					     syspath ) ;

		}
		zcmd = "" ;
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showDisks_partbuild( const string& tabName,
				   const string& disk,
				   string& sel,
				   string& part,
				   string& size,
				   string& pcent,
				   string& mount,
				   string& uuid,
				   string& label,
				   string& syspath )
{
	//
	// Build the lspf table with the disk partitions.
	//

	const char* path ;
	const char* blks ;
	const char* blksize ;

	struct udev_device* dev ;
	struct udev_device* dev_parent ;

	set<string> swaps ;

	tbcreate( tabName,
		  "",
		  "(SEL,PART,SIZE,PCENT,MOUNT,UUID,LABEL,SYSPATH)",
		  NOWRITE,
		  REPLACE ) ;

	struct udev* udev = udev_new() ;

	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	get_swaps( swaps ) ;

	struct udev_enumerate* enumerate = udev_enumerate_new( udev ) ;

	udev_enumerate_add_match_subsystem( enumerate, "block" ) ;
	udev_enumerate_scan_devices( enumerate ) ;

	struct udev_list_entry* devices = udev_enumerate_get_list_entry( enumerate ) ;
	struct udev_list_entry* entry ;

	udev_list_entry_foreach( entry, devices )
	{
		path = udev_list_entry_get_name( entry ) ;
		dev  = udev_device_new_from_syspath( udev, path ) ;
		if ( dev )
		{
			if ( udev_device_get_devnode( dev ) && string( udev_device_get_devtype( dev ) ) == "partition" )
			{
				part = udev_device_get_devnode( dev ) ;
				if ( abbrev( part, disk ) )
				{
					sel     = "" ;
					mount   = "" ;
					size    = "" ;
					pcent   = "" ;
					syspath = path ;
					get_mnt_uuid_label( part, mount, uuid, label ) ;
					if ( swaps.count( part ) > 0 )
					{
						mount = "[SWAP]" ;
					}
					dev_parent = udev_device_get_parent( dev ) ;
					if ( dev_parent )
					{
						blksize = udev_device_get_sysattr_value( dev_parent, "queue/physical_block_size" ) ;
						if ( blksize )
						{
							blks = udev_device_get_sysattr_value( dev, "size" ) ;
							if ( blks )
							{
								size = get_space( blksize, blks ) ;
							}
						}
					}
					pcent = get_pcent_space_used( mount ) ;
					tbadd( tabName ) ;
				}
			}
			udev_device_unref( dev ) ;
		}
	}

	udev_enumerate_unref( enumerate ) ;

	udev_unref( udev ) ;

	tbsort( tabName, "(PART,C,A)" ) ;
	tbtop( tabName ) ;
}


void psysutl::showDisks_mount( const string& partition )
{
	//
	// Mount a file system.
	//
	// Use 'mount' command if superuser and mount point has been entered.
	// Use 'udisksctl' for all other mounts.
	//

	int rc ;

	string part = partition ;
	string mpath ;
	string optro ;
	string optis ;
	string optsw ;
	string opts ;
	string optother ;

	bool use_mount_cmd ;

	string msg ;

	vector<string> results ;

	boost::system::error_code ec ;

	const string vlist1 = "PART MPATH OPTRO OPTIS OPTSW OPTOTHER" ;

	const char* use_mount     = "mount %s %s %s" ;
	const char* use_udisksctl = "udisksctl mount -b %s %s" ;

	switch ( get_user_type() )
	{
		case 0: use_mount_cmd = true ;
			break ;
		case 1: use_mount_cmd = false ;
			break ;
		case 2: use_mount_cmd = false ;
			use_udisksctl = "udisksctl mount --no-user-interaction -b %s %s" ;
			break ;
	}

	char buf[ 1024 ] ;

	vdefine( vlist1, &part, &mpath, &optro, &optis, &optsw, &optother ) ;

	addpop() ;
	while ( true )
	{
		display( "PSUT00DM", msg ) ;
		if ( RC > 0 ) { break ; }
		msg = "" ;
		opts  = ( optro == "/" ) ? "ro," : "" ;
		opts += ( optis == "/" ) ? "nosuid," : "" ;
		opts += ( optsw == "/" ) ? "sync," : "" ;
		opts += optother ;
		if ( opts != "" )
		{
			opts = "-o " + strip( opts, 'T', ',' ) ;
		}
		if ( use_mount_cmd && mpath != "" )
		{
			try
			{
				if ( !exists( mpath ) )
				{
					msg = "PSUT013E" ;
					continue ;
				}
			}
			catch ( const filesystem_error& ec )
			{
				set_message( msg, "PSUT013D", ec.what() ) ;
				continue ;
			}
			snprintf( buf,
				  sizeof( buf ),
				  use_mount,
				  partition.c_str(),
				  mpath.c_str(),
				  opts.c_str() ) ;
		}
		else
		{
			snprintf( buf,
				  sizeof( buf ),
				  use_udisksctl,
				  partition.c_str(),
				  opts.c_str() ) ;
		}

		execute_cmd( rc, buf, results ) ;
		if ( rc == 0 ) { break ; }
		set_message( msg, "PSUT013F", get_mount_error( rc ) ) ;
	}

	rempop() ;
	vdelete( vlist1 ) ;
}


void psysutl::showDisks_unmount( const string& partition )
{
	//
	// Unmount a file system using udisksctl.
	//

	int rc ;

	const char* use_udisksctl = "udisksctl unmount -b %s" ;

	char buf[ 1024 ] ;

	vector<string> results ;

	if ( get_user_type() == 2 )
	{
		use_udisksctl = "udisksctl unmount --no-user-interaction -b %s" ;
	}

	snprintf( buf,
		  sizeof( buf ),
		  use_udisksctl,
		  partition.c_str() ) ;

	execute_cmd( rc, buf, results ) ;
	if ( rc != 0 )
	{
		display_error( "PSUT013G", stderror ) ;
	}
}



/**************************************************************************************************************/
/**********************************              DISKSTATS                  ***********************************/
/**************************************************************************************************************/

void psysutl::showDEV_diskstats( const string& dev )
{
	//
	// Show /proc/diskstats for device dev.
	//

	string dev1 = dev.substr( dev.find_last_of( '/' ) + 1 ) ;

	string line ;
	string ignore ;

	string dstats1 ;
	string dstats2 ;
	string dstats3 = dev ;
	string dstats4 ;
	string dstats5 ;
	string dstats6 ;
	string dstats7 ;
	string dstats8 ;
	string dstats9 ;

	string dstats10 ;
	string dstats11 ;
	string dstats12 ;
	string dstats13 ;
	string dstats14 ;
	string dstats15 ;
	string dstats16 ;
	string dstats17 ;
	string dstats18 ;
	string dstats19 ;
	string dstats20 ;

	const string vlist1 = "DSTATS1  DSTATS2  DSTATS3  DSTATS4  DSTATS5  DSTATS6  DSTATS7  DSTATS8  DSTATS9  DSTATS10" ;
	const string vlist2 = "DSTATS11 DSTATS12 DSTATS13 DSTATS14 DSTATS15 DSTATS16 DSTATS17 DSTATS18 DSTATS19 DSTATS20" ;

	vdefine( vlist1, &dstats1, &dstats2, &dstats3, &dstats4, &dstats5, &dstats6, &dstats7, &dstats8, &dstats9, &dstats10 ) ;
	vdefine( vlist2, &dstats11, &dstats12, &dstats13, &dstats14, &dstats15, &dstats16, &dstats17, &dstats18, &dstats19, &dstats20 ) ;

	RC = 0 ;

	while ( RC == 0 )
	{
		std::ifstream if_diskstats( _PATH_PROC_DISKSTATS ) ;
		while ( getline( if_diskstats, line ) )
		{
			if ( word( line, 3 ) == dev1 )
			{
				stringstream ss( line ) ;
				ss >> dstats1  ;
				ss >> dstats2  ;
				ss >> ignore   ;
				ss >> dstats4  ;
				ss >> dstats5  ;
				ss >> dstats6  ;
				ss >> dstats7  ;
				ss >> dstats8  ;
				ss >> dstats9  ;
				ss >> dstats10 ;
				ss >> dstats11 ;
				ss >> dstats12 ;
				ss >> dstats13 ;
				ss >> dstats14 ;
				ss >> dstats15 ;
				ss >> dstats16 ;
				ss >> dstats17 ;
				ss >> dstats18 ;
				ss >> dstats19 ;
				ss >> dstats20 ;
				break ;
			}
		}
		if_diskstats.close() ;
		display( "PSUT00QI" ) ;
	}

	vdelete( vlist1, vlist2 ) ;
}


/**************************************************************************************************************/
/**********************************          SHOW RUNNING TASKS             ***********************************/
/**************************************************************************************************************/


void psysutl::showTasks()
{
	//
	// Show running tasks from 'top' output.
	//

	int rc ;
	int crp ;
	int csrrow ;
	int ztdtop ;
	int ztdsels ;

	string of ;
	string uf ;
	string csr ;
	string zcmd ;
	string sel ;
	string user ;
	string pid ;
	string nice ;
	string cpu ;
	string cpux ;
	string mem ;
	string memx ;
	string cmd ;
	string msg ;
	string table ;
	string panel ;

	vector<string> results ;

	const string vlist1 = "SEL USER PID NICE CPU CPUX MEM MEMX CMD" ;
	const string vlist2 = "USERF ONLYF" ;
	const string vlist3 = "CRP ZTDSELS ZTDTOP" ;

	vdefine( vlist1, &sel, &user, &pid, &nice, &cpu, &cpux, &mem, &memx, &cmd ) ;
	vdefine( vlist2, &uf, &of ) ;
	vdefine( vlist3, &crp, &ztdsels, &ztdtop ) ;

	table = "TSKL" + d2ds( taskid(), 4 ) ;

	showTasks_buildTable( table,
			      user,
			      pid,
			      nice,
			      cpu,
			      cpux,
			      mem,
			      cmd ) ;

	ztdtop  = 1 ;
	ztdsels = 0 ;
	msg     = "" ;
	csr     = "ZCMD" ;
	csrrow  = 0 ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( table ) ;
			tbskip( table, ztdtop ) ;
			panel = "PSUT00TK" ;
		}
		control( "DISPLAY", "REFRESH" ) ;
		tbdispl( table,
			 panel,
			 msg,
			 csr,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		msg    = "" ;
		zcmd   = "" ;
		panel  = "" ;
		csr    = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "K" )
		{
			rc = kill( ds2d( pid ), SIGKILL ) ;
			llog( "I", "Kill signal sent to PID " << pid << ".  RC=" << rc << endl ) ;
			if ( rc != 0 )
			{
				llog( "I", "ERROR: " << strerror( errno ) << endl ) ;
			}
		}
		else if ( sel == "T" )
		{
			rc = kill( ds2d( pid ), SIGTERM ) ;
			llog( "I", "Terminate signal sent to PID " << pid << ".  RC=" << rc << endl ) ;
			if ( rc != 0 )
			{
				llog( "I", "ERROR: " << strerror( errno ) << endl ) ;
			}
		}
		else if ( sel == "S" || sel == "I" )
		{
			select( "PGM(PCMD0A) PARM(systemctl status "+ pid +" )" ) ;
		}
		else if ( sel == "J" )
		{
			execute_cmd( rc, "systemctl whoami "+ pid, results ) ;
			if ( rc == 0 )
			{
				select( "PGM(PCMD0A) PARM(journalctl _PID="+ pid +" -b 0 --unit "+ results[ 0 ] +" ) SUSPEND" ) ;
			}
			else
			{
				select( "PGM(PCMD0A) PARM(journalctl _PID="+ pid +" -b 0 ) SUSPEND" ) ;
			}
		}
		if ( sel == "" && ztdsels < 2 )
		{
			showTasks_buildTable( table,
					      user,
					      pid,
					      nice,
					      cpu,
					      cpux,
					      mem,
					      cmd,
					      uf,
					      of ) ;
		}
	}

	tbend( table ) ;

	vdelete( vlist1, vlist2, vlist3 ) ;
}


void psysutl::showTasks_buildTable( const string& table,
				    string& user,
				    string& pid,
				    string& nice,
				    string& cpu,
				    string& cpux,
				    string& mem,
				    string& cmd,
				    const string& uf,
				    const string& of )
{
	//
	// Columns for top: PID, user, priority, NI, virt, RES, Size, Status, %CPU, %MEM, time, Command.
	//
	// Procedure can use 'top' or 'ps'.
	// top gives a better representation of CPU percentage but there is a delay.
	//
	// Do CONTROL DISPLAY REFRESH before TBDISPL when using 'top' as it seems to sometimes interfere with ncurses output.
	//

	int maxw1 ;
	int rc ;

	size_t p ;

	string excmd ;

	vector<string> results ;

	enum Columns
	{
		C_PID = 1,
		C_USER,
		C_PRI,
		C_NICE,
		C_VIRT,
		C_RES,
		C_SIZE,
		C_STATUS,
		C_CPU,
		C_MEM,
		C_TIME,
		C_CMD
	} ;

	vdefine( "ZSCRMAXW", &maxw1 ) ;
	vget( "ZSCRMAXW", SHARED ) ;

  //    excmd = "ps ax -o pid,user,pri,ni,vsz,drs,size,stat,%cpu,%mem,time,cmd" ;
	excmd = "top -b -n 1 -w " + d2ds( maxw1 ) ;

	execute_cmd( rc, excmd, results ) ;

	tbcreate( table,
		  "",
		  "(SEL,USER,PID,NICE,CPU,CPUX,MEM,MEMX,CMD)",
		  NOWRITE,
		  REPLACE ) ;

	for ( const auto& inLine : results )
	{
		tbvclear( table ) ;
		pid = word( inLine, C_PID ) ;
		if ( !datatype( pid, 'W' ) ) { continue ; }
		user = word( inLine, C_USER ) ;
		if ( uf != "" && uf != upper( user ) ) { continue ; }
		nice = word( inLine, C_NICE ) ;
		cpu  = word( inLine, C_CPU ) ;
		p    = cpu.find( '.' ) ;
		if ( p == string::npos ) { cpux = d2ds( ds2d( cpu ) * 10 ) ; }
		else                     { cpux = cpu ; cpux.erase( p, 1 ) ; }
		mem = word( inLine, C_MEM ) ;
		cmd = subword( inLine, C_CMD ) ;
		if ( of != "" && pos( of, upper( cmd ) ) == 0 ) { continue ; }
		trim_left( cmd ) ;
		cmd = strip( strip( cmd, 'L', '`' ), 'L', '-' ) ;
		trim_left( cmd ) ;
		tbadd( table ) ;
	}

	tbsort( table, "(CPUX,N,D)" ) ;
	tbtop( table ) ;

	vdelete( "ZSCRMAXW" ) ;
}



/**************************************************************************************************************/
/**********************************            USB SYBSYSTEM                ***********************************/
/**************************************************************************************************************/

void psysutl::showUSB()
{
	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	string tabName ;
	string panel ;
	string cursor ;

	string sel ;
	string manuf ;
	string serial ;
	string prod ;
	string vid ;
	string pid ;
	string syspath ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "SEL VID PID PROD MANUF SERIAL SYSPATH" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &sel, &vid, &pid, &prod, &manuf, &serial, &syspath ) ;

	tabName = "USB" + d2ds( taskid(), 5 ) ;

	showUSB_build( tabName,
		       sel,
		       vid,
		       pid,
		       prod,
		       serial,
		       manuf,
		       syspath ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00U1" ;
		}
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "P" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_properties( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "T" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_sysattrs( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		if ( ztdsels < 2 )
		{
			showUSB_build( tabName,
				       sel,
				       vid,
				       pid,
				       prod,
				       serial,
				       manuf,
				       syspath ) ;
		}
	}

	tbend( tabName ) ;

	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showUSB_build( const string& tabName,
			     string& sel,
			     string& vid,
			     string& pid,
			     string& prod,
			     string& serial,
			     string& manuf,
			     string& syspath )
{
	//
	// List USB devices.
	//

	struct udev* udev = udev_new() ;

	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	tbcreate( tabName,
		  "",
		  "(SEL,MANUF,PROD,SERIAL,VID,PID,SYSPATH)",
		  NOWRITE,
		  REPLACE ) ;

	struct udev_enumerate* enumerate = udev_enumerate_new( udev ) ;

	udev_enumerate_add_match_subsystem( enumerate, "usb" ) ;

	udev_enumerate_scan_devices( enumerate ) ;

	struct udev_list_entry* devices = udev_enumerate_get_list_entry( enumerate ) ;
	struct udev_list_entry* entry ;

	udev_list_entry_foreach( entry, devices )
	{
		const char* path = udev_list_entry_get_name( entry ) ;
		struct udev_device* dev = udev_device_new_from_syspath( udev, path ) ;
		if ( dev )
		{
			if ( udev_device_get_devnode( dev ) )
			{
				sel     = "" ;
				syspath = path ;
				get_sysattr( dev, "manufacturer", manuf, "0000" ) ;
				get_sysattr( dev, "idVendor", vid, "0000" ) ;
				get_sysattr( dev, "idProduct", pid, "0000" ) ;
				get_sysattr( dev, "product", prod, "0000" ) ;
				get_sysattr( dev, "serial", serial, "0000" ) ;
				tbadd( tabName ) ;
			}
			udev_device_unref( dev ) ;
		}
	}

	udev_enumerate_unref( enumerate ) ;

	udev_unref( udev ) ;

	tbsort( tabName, "(MANUF,C,A)" ) ;
	tbtop( tabName ) ;
}


/**************************************************************************************************************/
/**********************************          NETWORK SYBSYSTEM              ***********************************/
/**************************************************************************************************************/

void psysutl::showNetwork()
{
	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	string tabName ;
	string cursor ;
	string panel ;
	string zcmd ;

	string sel ;
	string netif ;
	string nettype ;
	string netstat ;
	string ipaddr ;
	string ipaddr6 ;
	string macaddr ;
	string netmask ;
	string netmask6 ;
	string bcaddr  ;
	string bcaddr6 ;
	string dstaddr ;
	string dstaddr6 ;
	string syspath ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD SEL NETIF NETTYPE NETSTAT IPADDR IPADDR6 MACADDR BCADDR BCADDR6" ;
	const string vlist3 = "DSTADDR DSTADDR6 NETMASK NETMASK6 SYSPATH" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &sel, &netif, &nettype, &netstat, &ipaddr, &ipaddr6, &macaddr, &bcaddr, &bcaddr6 ) ;
	vdefine( vlist3, &dstaddr, &dstaddr6, &netmask, &netmask6, &syspath ) ;

	tabName = "NET" + d2ds( taskid(), 5 ) ;

	showNetwork_build( tabName,
			   sel,
			   netif,
			   nettype,
			   netstat,
			   ipaddr,
			   ipaddr6,
			   macaddr,
			   netmask,
			   netmask6,
			   bcaddr,
			   bcaddr6,
			   dstaddr,
			   dstaddr6,
			   syspath ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00N1" ;
		}
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( zcmd == "NETHOGS" )
		{
			zcmd = "" ;
			control( "DISPLAY", "SAVE" ) ;
			showNetwork_usage() ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		if ( sel == "I" )
		{
			showNetwork_info( netif, syspath ) ;
		}
		else if ( sel == "P" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_properties( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "S" )
		{
			control( "DISPLAY", "SAVE" ) ;
			showNetwork_usage( netif ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "T" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_sysattrs( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		if ( ztdsels < 2 )
		{
			showNetwork_build( tabName,
					   sel,
					   netif,
					   nettype,
					   netstat,
					   ipaddr,
					   ipaddr6,
					   macaddr,
					   netmask,
					   netmask6,
					   bcaddr,
					   bcaddr6,
					   dstaddr,
					   dstaddr6,
					   syspath ) ;
		}
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2, vlist3 ) ;
}


void psysutl::showNetwork_build( const string& tabName,
				 string& sel,
				 string& netif,
				 string& nettype,
				 string& netstat,
				 string& ipaddr,
				 string& ipaddr6,
				 string& macaddr,
				 string& netmask,
				 string& netmask6,
				 string& bcaddr,
				 string& bcaddr6,
				 string& dstaddr,
				 string& dstaddr6,
				 string& syspath )
{
	//
	// List Network interfaces.
	//

	map<string, if_dtls> if_details ;

	struct udev* udev = udev_new() ;

	struct udev_device* dev ;

	struct udev_enumerate* enumerate ;

	struct udev_list_entry* entry ;
	struct udev_list_entry* devices ;

	const char* path ;
	const char* value ;

	const string names = "(SEL,NETIF,NETTYPE,NETSTAT,IPADDR,IPADDR6,MACADDR,BCADDR,BCADDR6,DSTADDR,DSTADDR6,GADDR,GADDR6,NETMASK,NETMASK6,SYSPATH)" ;

	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	tbcreate( tabName,
		  "",
		  names,
		  NOWRITE,
		  REPLACE ) ;

	showNetwork_get_details( if_details ) ;

	enumerate = udev_enumerate_new( udev ) ;

	udev_enumerate_add_match_subsystem( enumerate, "net" ) ;

	udev_enumerate_scan_devices( enumerate ) ;

	devices = udev_enumerate_get_list_entry( enumerate ) ;

	udev_list_entry_foreach( entry, devices )
	{
		tbvclear( tabName ) ;
		path  = udev_list_entry_get_name( entry ) ;
		dev   = udev_device_new_from_syspath( udev, path ) ;
		value = udev_device_get_property_value( dev, "INTERFACE" ) ;
		if ( !value )
		{
			continue ;
		}
		netif   = value ;
		syspath = path ;
		auto it = if_details.find( netif ) ;
		if ( it != if_details.end() )
		{
			nettype  = it->second.nettype ;
			netstat  = it->second.netstat ;
			ipaddr   = it->second.ipaddr ;
			ipaddr6  = it->second.ipaddr6 ;
			macaddr  = lower( it->second.macaddr ) ;
			if ( it->second.point2point )
			{
				vreplace( "GADDR", it->second.dstaddr ) ;
				vreplace( "GADDR6", it->second.dstaddr6 ) ;
			}
			else
			{
				vreplace( "GADDR", it->second.bcaddr ) ;
				vreplace( "GADDR6", it->second.bcaddr6 ) ;
			}
			bcaddr   = it->second.bcaddr ;
			bcaddr6  = it->second.bcaddr6 ;
			dstaddr  = it->second.dstaddr ;
			dstaddr6 = it->second.dstaddr6 ;
			netmask  = it->second.netmask ;
			netmask6 = it->second.netmask6 ;
		}
		tbadd( tabName ) ;
		udev_device_unref( dev ) ;
	}

	udev_enumerate_unref( enumerate ) ;

	udev_unref( udev ) ;

	tbsort( tabName, "(NETIF,C,A)" ) ;
	tbtop( tabName ) ;
}


void psysutl::showNetwork_get_details( map<string, if_dtls>& if_details )
{
	//
	// Get interface details and store in map if_details.
	//

	struct ifaddrs* ifaddr ;
	struct ifaddrs* ifa ;

	void* addr ;

	if ( getifaddrs( &ifaddr ) == -1 )
	{
		return ;
	}

	for ( ifa = ifaddr ; ifa ; ifa = ifa->ifa_next )
	{
		if ( !ifa->ifa_addr ) { continue ; }
		auto it = if_details.find( ifa->ifa_name ) ;
		if ( it == if_details.end() )
		{
			auto result = if_details.insert( pair<string, if_dtls>( ifa->ifa_name, if_dtls() ) ) ;
			it = result.first ;
		}
		if ( ifa->ifa_flags & IFF_UP )
		{
			it->second.netstat = add_str( it->second.netstat, ( ifa->ifa_flags & IFF_UP ) ? "Up" : "Down" ) ;
		}
		if ( ifa->ifa_flags & IFF_RUNNING )
		{
			it->second.netstat = add_str( it->second.netstat, "Running" ) ;
		}
		if ( ifa->ifa_flags & IFF_LOOPBACK )
		{
			it->second.nettype = add_str( it->second.nettype, "Loopback" ) ;
		}
		if ( ifa->ifa_flags & IFF_POINTOPOINT )
		{
			it->second.nettype = add_str( it->second.nettype, "Point to Point" ) ;
		}
		if ( ifa->ifa_flags & IFF_BROADCAST )
		{
			it->second.nettype = add_str( it->second.nettype, "Broadcast" ) ;
		}
		if ( ifa->ifa_flags & IFF_MULTICAST )
		{
			it->second.nettype = add_str( it->second.nettype, "Multicast" ) ;
		}
		if ( ifa->ifa_addr->sa_family == AF_INET )
		{
			addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr ;
			it->second.ipaddr = ip_to_string( ifa->ifa_addr->sa_family, addr ) ;
		}
		else
		{
			addr = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr ;
			it->second.ipaddr6 = ip_to_string( ifa->ifa_addr->sa_family, addr ) ;
		}
		if ( ifa->ifa_netmask )
		{
			if ( ifa->ifa_netmask->sa_family == AF_INET )
			{
				addr = &((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr ;
				it->second.netmask = ip_to_string( ifa->ifa_netmask->sa_family, addr ) ;
			}
			else
			{
				addr = &((struct sockaddr_in6*)ifa->ifa_netmask)->sin6_addr ;
				it->second.netmask6 = ip_to_string( ifa->ifa_netmask->sa_family, addr ) ;
			}

		}
		if ( ifa->ifa_ifu.ifu_broadaddr )
		{
			if ( ifa->ifa_flags & IFF_POINTOPOINT)
			{
				it->second.point2point = true ;
				if ( ifa->ifa_ifu.ifu_dstaddr->sa_family == AF_INET )
				{
					addr = &((struct sockaddr_in*)ifa->ifa_ifu.ifu_dstaddr)->sin_addr ;
					it->second.dstaddr = ip_to_string( ifa->ifa_ifu.ifu_dstaddr->sa_family, addr ) ;
				}
				else
				{
					addr = &((struct sockaddr_in6*)ifa->ifa_ifu.ifu_dstaddr)->sin6_addr ;
					it->second.dstaddr6 = ip_to_string( ifa->ifa_ifu.ifu_dstaddr->sa_family, addr ) ;
				}
			}
			else
			{
				if ( ifa->ifa_ifu.ifu_broadaddr->sa_family == AF_INET )
				{
					addr = &((struct sockaddr_in*)ifa->ifa_ifu.ifu_broadaddr)->sin_addr ;
					it->second.bcaddr = ip_to_string( ifa->ifa_ifu.ifu_broadaddr->sa_family, addr ) ;
				}
				else
				{
					addr = &((struct sockaddr_in6*)ifa->ifa_ifu.ifu_broadaddr)->sin6_addr ;
					it->second.bcaddr6 = ip_to_string( ifa->ifa_ifu.ifu_broadaddr->sa_family, addr ) ;
				}
			}
		}
		if ( ifa->ifa_addr->sa_family == AF_PACKET )
		{
			struct sockaddr_ll* s = (struct sockaddr_ll*)ifa->ifa_addr ;
			for ( int i = 0 ; i < s->sll_halen ; ++i )
			{
				it->second.macaddr += c2xs( s->sll_addr[ i ] ) ;
				if ( i != s->sll_halen - 1 )
				{
					it->second.macaddr.push_back( ':' ) ;
				}
			}
		}
	}

	freeifaddrs( ifaddr ) ;
}


#ifdef WITH_NETHOGS
void psysutl::showNetwork_usage( const string& netif )
{
	//
	// Show interface usage such as processes, upload/download rates, etc.
	// This uses libnethogs to get the data.
	//
	// Redirect STDERR to a pipe to get errors from libnethogs.  Restore on return.
	//
	// Temporarily redirect STDOUT to a pipe to suppress messages while starting libnethogs.
	// Restore when started.
	//

	char buf[ 128 ] ;

	int fd1 ;
	int fd2 ;
	int retc ;

	int my_pipe1[ 2 ] ;
	int my_pipe2[ 2 ] ;

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	uint64_t tot_sent = 0 ;
	uint64_t tot_recv = 0 ;

	string zcmd ;

	string tabName ;
	string cursor ;
	string panel ;
	string temp ;

	string sel ;
	string recid ;
	string pid ;
	string user ;
	string device ;
	string name ;
	string program ;
	string sent ;
	string recv ;
	string sendr ;
	string recvr ;
	string sendt ;
	string recvt ;
	string sendrt ;
	string recvrt ;

	if ( nethogs_running )
	{
		setmsg( "PSUT015A" ) ;
		return ;
	}

	row_updates.clear() ;

	if ( pipe2( my_pipe1, O_NONBLOCK ) == -1 )
	{
		display_error( "PSUT015C", strerror( errno ) ) ;
		return ;
	}

	if ( pipe2( my_pipe2, O_NONBLOCK ) == -1 )
	{
		close( my_pipe1[ 0 ] ) ;
		close( my_pipe1[ 1 ] ) ;
		display_error( "PSUT015C", strerror( errno ) ) ;
		return ;
	}

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD SEL RECID PID USER DEVICE NAME PROGRAM SENT RECV SENDR RECVR" ;
	const string vlist3 = "SENDT RECVT SENDRT RECVRT" ;

	tabName = "NTU" + d2ds( taskid(), 5 ) ;

	control( "ABENDRTN", static_cast<void (pApplication::*)()>(&psysutl::showNetwork_usage_cleanup) ) ;

	fd1 = dup( STDERR_FILENO ) ;
	dup2( my_pipe1[ 1 ], STDERR_FILENO ) ;

	fd2 = dup( STDOUT_FILENO ) ;
	dup2( my_pipe2[ 1 ], STDOUT_FILENO ) ;

	coll_status = CL_STARTING ;

	bThread = new boost::thread( &psysutl::showNetwork_usage_collector, this ) ;

	do
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 100 ) ) ;
	}
	while ( coll_status == CL_STARTING ) ;

	fflush( stdout ) ;
	close( my_pipe2[ 0 ] ) ;
	close( my_pipe2[ 1 ] ) ;
	dup2( fd2, STDOUT_FILENO ) ;

	if ( coll_status == CL_STOPPED )
	{
		fflush( stderr ) ;
		close( my_pipe1[ 1 ] ) ;
		temp = "" ;
		retc = read( my_pipe1[ 0 ], buf, sizeof( buf ) ) ;
		while ( retc != -1 )
		{
			temp += string( buf, retc ) ;
			retc = read( my_pipe1[ 0 ], buf, sizeof( buf ) ) ;
		}
		close( my_pipe1[ 0 ] ) ;
		dup2( fd1, STDERR_FILENO ) ;
		temp = translate( temp, 0x20, 0x0a ) ;
		llog( "O", "STDERR " << temp << endl ) ;
		display_error( "PSUT015B", temp ) ;
		delete bThread ;
		return ;
	}

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &sel, &recid, &pid, &user, &device, &name, &program, &sent, &recv, &sendr, &recvr ) ;
	vdefine( vlist3, &sendt, &recvt, &sendrt, &recvrt ) ;

	nethogs_running = true ;

	const string names = "(SEL,PID,USER,DEVICE,NAME,PROGRAM,SENT,RECV,#SENT,#RECV,SENDR,RECVR,#SENDR,#RECVR)" ;

	tbcreate( tabName,
		  "RECID",
		  names,
		  NOWRITE,
		  REPLACE ) ;

	showNetwork_usage_build( tabName,
				 netif,
				 sel,
				 recid,
				 pid,
				 user,
				 device,
				 name,
				 program,
				 sent,
				 recv,
				 sendr,
				 recvr,
				 sendrt,
				 recvrt,
				 tot_sent,
				 tot_recv ) ;

	sendt = d2size( tot_sent ) ;
	recvt = d2size( tot_recv ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00ND" ;
		}
		control( "DISPLAY", "REFRESH" ) ;
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "K" )
		{
			retc = kill( ds2d( pid ), SIGKILL ) ;
			llog( "I", "Kill signal sent to PID " << pid << ".  RC=" << retc << endl ) ;
			if ( retc != 0 )
			{
				display_error( "PSUT015D", strerror( errno ) ) ;
			}
		}
		else if ( sel == "S" )
		{
			select( "PGM(PCMD0A) PARM(systemctl status "+ pid +" ) SUSPEND" ) ;
		}
		else if ( sel == "T" )
		{
			retc = kill( ds2d( pid ), SIGTERM ) ;
			llog( "I", "Terminate signal sent to PID " << pid << ".  RC=" << retc << endl ) ;
			if ( retc != 0 )
			{
				display_error( "PSUT015D", strerror( errno ) ) ;
			}
		}
		if ( ztdsels < 2 )
		{
			showNetwork_usage_build( tabName,
						 netif,
						 sel,
						 recid,
						 pid,
						 user,
						 device,
						 name,
						 program,
						 sent,
						 recv,
						 sendr,
						 recvr,
						 sendrt,
						 recvrt,
						 tot_sent,
						 tot_recv ) ;
			sendt = d2size( tot_sent ) ;
			recvt = d2size( tot_recv ) ;
		}
	}

	nethogsmonitor_breakloop() ;

	while ( coll_status != CL_STOPPED )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	delete bThread ;

	nethogs_running = false ;

	fflush( stderr ) ;
	close( my_pipe1[ 0 ] ) ;
	close( my_pipe1[ 1 ] ) ;
	dup2( fd1, STDERR_FILENO ) ;

	tbend( tabName ) ;
	vdelete( vlist1, vlist2, vlist3 ) ;
}


void psysutl::showNetwork_usage_build( const string& tabName,
				       const string& netif,
				       string& sel,
				       string& recid,
				       string& pid,
				       string& user,
				       string& device,
				       string& name,
				       string& program,
				       string& sent,
				       string& recv,
				       string& sendr,
				       string& recvr,
				       string& sendrt,
				       string& recvrt,
				       uint64_t& tot_sent,
				       uint64_t& tot_recv )
{
	//
	// Show detailed network information for interface netif.
	//

	float tot_sent_kbs ;
	float tot_recv_kbs ;

	boost::lock_guard<boost::mutex> lock( mtxg ) ;

	tbvclear( tabName ) ;
	for ( auto& row : row_updates )
	{
		const NethogsMonitorRecord& row_data = row.second.record ;
		if ( netif != "" && netif != row_data.device_name )
		{
			continue ;
		}
		recid = to_string( row_data.record_id ) ;
		if( row.second.action == NETHOGS_APP_ACTION_REMOVE )
		{
			tbdelete( tabName ) ;
		}
		else
		{
			tbget( tabName ) ;
			tot_sent += row_data.sent_bytes ;
			tot_recv += row_data.recv_bytes ;
			if ( RC == 0 )
			{
				tot_recv -= stoull( get_dialogue_var( "#RECV" ) ) ;
				tot_sent -= stoull( get_dialogue_var( "#SENT" ) ) ;
			}
			pid      = d2ds( row_data.pid ) ;
			user     = get_username( row_data.uid ) ;
			device   = row_data.device_name ;
			vreplace( "#RECV", to_string( row_data.recv_bytes ) ) ;
			vreplace( "#SENT", to_string( row_data.sent_bytes ) ) ;
			recv     = d2size( row_data.recv_bytes ) ;
			sent     = d2size( row_data.sent_bytes ) ;
			sendr    = format_kbs( row_data.sent_kbs ) ;
			recvr    = format_kbs( row_data.recv_kbs ) ;
			vreplace( "#SENDR", to_string( row_data.sent_kbs ) ) ;
			vreplace( "#RECVR", to_string( row_data.recv_kbs ) ) ;
			name     = get_program_name( row_data.name ) ;
			program  = row_data.name ;
			tbmod( tabName ) ;
			tbvclear( tabName ) ;
		}
	}

	row_updates.clear() ;

	tbsort( tabName, "(PID,N,A)" ) ;
	tbtop( tabName ) ;

	tot_sent_kbs = 0 ;
	tot_recv_kbs = 0 ;

	tbskip( tabName ) ;
	while ( RC == 0 )
	{
		tot_sent_kbs += stof( get_dialogue_var( "#SENDR" ) ) ;
		tot_recv_kbs += stof( get_dialogue_var( "#RECVR" ) ) ;
		tbskip( tabName ) ;
	}

	sendrt = format_kbs( tot_sent_kbs ) ;
	recvrt = format_kbs( tot_recv_kbs ) ;

	tbtop( tabName ) ;
}


void psysutl::showNetwork_usage_collector()
{
	//
	// This method runs in the background to perform a nethogsmonitor_loop().
	//

	coll_status = CL_RUNNING ;

	nethogsmonitor_loop( psysutl::onNethogsUpdate, nullptr, 1000 ) ;

	coll_status = CL_STOPPED ;
}


void psysutl::onNethogsUpdate( int action, NethogsMonitorRecord const* record )
{
	boost::lock_guard<boost::mutex> lock( mtxg ) ;

	row_updates[ record->record_id ] = Update( action, *record ) ;
}


void psysutl::showNetwork_usage_cleanup()
{
	//
	// Stop the background data collector when an abend in the main thread occurs.
	//

	if ( coll_status == CL_RUNNING )
	{
		nethogsmonitor_breakloop() ;
		while ( coll_status != CL_STOPPED )
		{
			boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
		}
	}

	delete bThread ;

	nethogs_running = false ;
}

#else
void psysutl::showNetwork_usage( const string& netif )
{
	//
	// Issue message if built without nethogs.
	//

       setmsg( "PSUT015E" ) ;
}
#endif

void psysutl::showNetwork_info( const string& netif,
				const string& syspath )
{
	//
	// Show interface information.
	//

	string zcmd ;

	string ivendor ;
	string imodel ;
	string idriver ;
	string iclass ;
	string isclass ;

	string itxp ;
	string itxb ;
	string itxe ;
	string itxd ;
	string irxp ;
	string irxb ;
	string irxe ;
	string irxd ;

	struct udev_device* dev ;

	const string vlist1 = "ZCMD IVENDOR IMODEL IDRIVER ICLASS ISCLASS" ;
	const string vlist2 = "ITXP ITXB ITXE ITXD IRXP IRXB IRXE IRXD" ;

	vdefine( vlist1, &zcmd, &ivendor, &imodel, &idriver, &iclass, &isclass ) ;
	vdefine( vlist2, &itxp, &itxb, &itxe, &itxd, &irxp, &irxb, &irxe, &irxd ) ;

	struct udev* udev = udev_new() ;
	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	dev = udev_device_new_from_syspath( udev, syspath.c_str() ) ;

	get_property( dev, "ID_VENDOR_FROM_DATABASE", ivendor ) ;
	get_property( dev, "ID_MODEL_FROM_DATABASE", imodel ) ;
	get_property( dev, "ID_NET_DRIVER", idriver ) ;
	get_property( dev, "ID_PCI_CLASS_FROM_DATABASE", iclass ) ;
	get_property( dev, "ID_PCI_SUBCLASS_FROM_DATABASE", isclass ) ;

	udev_device_unref( dev ) ;
	udev_unref( udev ) ;

	RC = 0 ;
	while ( RC == 0 )
	{
		showNetwork_stats( netif, itxp, itxb, itxe, itxd, irxp, irxb, irxe, irxd ) ;
		display( "PSUT00NI" ) ;
	}

	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showNetwork_stats( const string& netif,
				 string& itxp,
				 string& itxb,
				 string& itxe,
				 string& itxd,
				 string& irxp,
				 string& irxb,
				 string& irxe,
				 string& irxd )
{
	//
	// Get interface transmit/receive/error/drop stats.
	//
	// Use /proc/net/dev to get the statistics as it uses rtnl_link_stats64 internally.
	// ifa_data uses rtnl_link_stats so will soon wrap.
	//

	string line ;
	string key = netif + ":" ;

	std::ifstream if_devnet( _PATH_PROC_NETDEV ) ;

	getline( if_devnet, line ) ;
	getline( if_devnet, line ) ;

	while ( getline( if_devnet, line ) )
	{
		if ( word( line, 1 ) == key )
		{
			itxp = word( line, 11 ) ;
			itxb = word( line, 10 ) ;
			itxe = word( line, 12 ) ;
			itxd = word( line, 13 ) ;
			irxp = word( line, 3 ) ;
			irxb = word( line, 2 ) ;
			irxe = word( line, 4 ) ;
			irxd = word( line, 5 ) ;
			break ;
		}
	}

	if_devnet.close() ;
}


/**************************************************************************************************************/
/**********************************            PCI DEVICES                  ***********************************/
/**************************************************************************************************************/

void psysutl::showPCI()
{
	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	string tabName ;
	string cursor ;
	string panel ;

	string sel ;
	string slot ;
	string name ;
	string pciid ;
	string dbclass ;
	string dbsclass ;
	string vendor ;
	string syspath ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "SEL SLOT NAME PCIID DBCLASS DBSCLASS VENDOR SYSPATH" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &sel, &slot, &name, &pciid, &dbclass, &dbsclass, &vendor, &syspath ) ;

	tabName = "PCI" + d2ds( taskid(), 5 ) ;

	showPCI_build( tabName,
		       sel,
		       slot,
		       name,
		       pciid,
		       dbclass,
		       dbsclass,
		       vendor,
		       syspath ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00P1" ;
		}
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "P" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_properties( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "T" )
		{
			control( "DISPLAY", "SAVE" ) ;
			DEV_list_all_sysattrs( syspath ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		if ( ztdsels < 2 )
		{
			showPCI_build( tabName,
				       sel,
				       slot,
				       name,
				       pciid,
				       dbclass,
				       dbsclass,
				       vendor,
				       syspath ) ;
		}
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showPCI_build( const string& tabName,
			     string& sel,
			     string& slot,
			     string& name,
			     string& pciid,
			     string& dbclass,
			     string& dbsclass,
			     string& vendor,
			     string& syspath )
{
	//
	// Build PCI device list from udev.
	//

	vector<path> vt ;

	struct udev_device* dev ;

	struct udev* udev = udev_new() ;
	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	tbcreate( tabName,
		  "",
		  "(SEL,SLOT,PCIID,DBCLASS,DBSCLASS,NAME,VENDOR,SYSPATH)",
		  NOWRITE,
		  REPLACE ) ;

	try
	{
		copy( directory_iterator( _PATH_SYS_PCIDEVS ), directory_iterator(), back_inserter( vt ) ) ;
	}
	catch (...)
	{
		return ;
	}

	tbvclear( tabName ) ;
	for ( auto& ent : vt )
	{
		dev = udev_device_new_from_syspath( udev, ent.string().c_str() ) ;
		get_property( dev, "PCI_SLOT_NAME", slot ) ;
		get_property( dev, "PCI_ID", pciid ) ;
		get_property( dev, "ID_MODEL_FROM_DATABASE", name ) ;
		get_property( dev, "ID_VENDOR_FROM_DATABASE", vendor ) ;
		get_property( dev, "ID_PCI_CLASS_FROM_DATABASE", dbclass ) ;
		get_property( dev, "ID_PCI_SUBCLASS_FROM_DATABASE", dbsclass ) ;
		syspath = ent.string() ;
		tbadd( tabName ) ;
		tbvclear( tabName ) ;
		udev_device_unref( dev ) ;
	}

	udev_unref( udev ) ;

	tbsort( tabName, "(SLOT,C,A)" ) ;
	tbtop( tabName ) ;
}


/**************************************************************************************************************/
/**********************************          LIST UDEV PROPERTIES           ***********************************/
/**************************************************************************************************************/


void psysutl::DEV_list_all_properties( const string& syspath )
{
	//
	// List all property/value pairs for a given syspath.
	//

	int ztdtop  = 1 ;

	string tabName = "UDV" + d2ds( taskid(), 5 ) ;

	string property ;
	string value ;
	string inclpd ;

	string zcmd ;

	int pathl ;
	int maxw ;

	const string vlist1 = "ZCMD INCLPD PROPERTY VALUE" ;
	const string vlist2 = "PATHL ZSCRMAXW ZTDTOP" ;

	vdefine( vlist1, &zcmd, &inclpd, &property, &value ) ;
	vdefine( vlist2, &pathl, &maxw, &ztdtop ) ;

	vget( "ZSCRMAXW INCLPD", SHARED ) ;
	pathl = maxw - 19 ;

	RC = 0 ;
	while ( RC == 0 )
	{
		tbcreate( tabName,
			  "",
			  "(PROPERTY,VALUE)",
			  NOWRITE,
			  REPLACE ) ;
		DEV_list_all_properties_add( syspath, tabName, property, value, syspath.size(), ( inclpd == "/" ) ) ;
		tbsort( tabName, "(PROPERTY,C,A)" ) ;
		tbtop( tabName ) ;
		tbskip( tabName, ztdtop ) ;
		tbdispl( tabName, "PSUT00DV" ) ;
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2 ) ;
}


void psysutl::DEV_list_all_properties_add( const string& syspath,
					   const string& tabName,
					   string& property,
					   string& value,
					   size_t l,
					   bool descend )
{
	//
	// List all property/value pairs for a given syspath.
	// Recursively descend into subdirectories, if specified, ignoring symlinks.
	//

	vector<path> vt ;

	const char* text1 ;
	const char* text2 ;

	struct udev_list_entry* list_entry ;

	struct udev* udev = udev_new() ;
	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	struct udev_device* dev = udev_device_new_from_syspath( udev, syspath.c_str() ) ;

	udev_list_entry_foreach( list_entry, udev_device_get_properties_list_entry( dev ) )
	{
		text1 = udev_list_entry_get_name( list_entry ) ;
		text2 = udev_list_entry_get_value( list_entry ) ;
		if ( text2 )
		{
			property = ( syspath.size() == l ) ? text1 : syspath.substr( l ) + "/" + text1 ;
			value    = text2 ;
			tbadd( tabName ) ;
		}
	}

	udev_device_unref( dev ) ;
	udev_unref( udev ) ;

	if ( descend )
	{
		try
		{
			copy( directory_iterator( syspath ), directory_iterator(), back_inserter( vt ) ) ;
		}
		catch (...)
		{
			return ;
		}
		for ( auto& ent : vt )
		{
			if ( !is_symlink( ent ) && is_directory( ent ) )
			{
				DEV_list_all_properties_add( ent.string(), tabName, property, value, l, descend ) ;
			}
		}
	}
}


/**************************************************************************************************************/
/**********************************           LIST UDEV SYSATTRS            ***********************************/
/**************************************************************************************************************/


void psysutl::DEV_list_all_sysattrs( const string& syspath )
{
	//
	// List all sysattrs for a given syspath.
	//

	int ztdtop  = 1 ;

	string tabName = "SAR" + d2ds( taskid(), 5 ) ;

	string zcmd ;

	string sysattr ;
	string value ;
	string inclsd ;

	int pathl ;
	int maxw ;

	const string vlist1 = "ZCMD INCLSD SYSATTR VALUE" ;
	const string vlist2 = "PATHL ZSCRMAXW ZTDTOP" ;

	vdefine( vlist1, &zcmd, &inclsd, &sysattr, &value ) ;
	vdefine( vlist2, &pathl, &maxw, &ztdtop ) ;

	vget( "ZSCRMAXW INCLSD", SHARED ) ;
	pathl = maxw - 19 ;

	RC = 0 ;
	while ( RC == 0 )
	{
		tbcreate( tabName,
			  "",
			  "(SYSATTR,VALUE)",
			  NOWRITE,
			  REPLACE ) ;
		DEV_list_all_sysattrs_add( syspath, tabName, sysattr, value, syspath.size(), ( inclsd == "/" ) ) ;
		tbsort( tabName, "(SYSATTR,C,A)" ) ;
		tbtop( tabName ) ;
		tbskip( tabName, ztdtop ) ;
		tbdispl( tabName, "PSUT00TV" ) ;
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2 ) ;
}


void psysutl::DEV_list_all_sysattrs_add( const string& syspath,
					 const string& tabName,
					 string& sysattr,
					 string& value,
					 size_t l,
					 bool descend )
{
	//
	// List all sysattr/value pairs for a given syspath.
	// Recursively descend into subdirectories, if specified, ignoring symlinks.
	//

	vector<path> vt ;

	const char* text1 ;
	const char* text2 ;

	struct udev_list_entry* list_entry ;

	struct udev* udev = udev_new() ;
	if ( !udev )
	{
		setmsg( "PSUT011D" ) ;
		return ;
	}

	struct udev_device* dev = udev_device_new_from_syspath( udev, syspath.c_str() ) ;

	udev_list_entry_foreach( list_entry, udev_device_get_sysattr_list_entry( dev ) )
	{
		text1   = udev_list_entry_get_name( list_entry ) ;
		sysattr = ( syspath.size() == l ) ? text1 : syspath.substr( l ) + "/" + text1 ;
		text2   = udev_device_get_sysattr_value( dev, text1 ) ;
		if ( text2 )
		{
			value = text2 ;
		}
		tbadd( tabName ) ;
	}

	udev_device_unref( dev ) ;
	udev_unref( udev ) ;

	if ( descend )
	{
		try
		{
			copy( directory_iterator( syspath ), directory_iterator(), back_inserter( vt ) ) ;
		}
		catch (...)
		{
			return ;
		}
		for ( auto& ent : vt )
		{
			if ( !is_symlink( ent ) && is_directory( ent ) )
			{
				DEV_list_all_sysattrs_add( ent.string(), tabName, sysattr, value, l, descend ) ;
			}
		}
	}
}


/**************************************************************************************************************/
/**********************************          SYSTEMD DISPLAY                ***********************************/
/**************************************************************************************************************/


void psysutl::showSystemdInfo()
{
	//
	// Display and manage systemd units.
	//

	string zcmd ;
	string zverb ;
	string panel ;
	string parms ;
	string tabName ;

	string filter1 ;
	string filter2 ;
	string filter3 ;
	string filter4 ;
	string filter5 ;
	string filter6 ;
	string filter7 ;

	string filtro1 ;
	string filtro2 ;
	string filtro3 ;
	string filtro4 ;
	string filtro5 ;
	string filtro6 ;
	string filtro7 ;

	string sel ;
	string unit ;

	string cursor ;

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;
	int ppos    = 0 ;

	int rc ;
	int crp ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD ZVERB SEL UNIT PARMS" ;
	const string vlist3 = "FILTER1 FILTER2 FILTER3 FILTER4 FILTER5 FILTER6 FILTER7" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &zverb, &sel, &unit, &parms ) ;
	vdefine( vlist3, &filter1, &filter2, &filter3, &filter4, &filter5, &filter6, &filter7 ) ;

	vget( vlist3, PROFILE ) ;

	filtro1 = filter1 ;
	filtro2 = filter2 ;
	filtro3 = filter3 ;
	filtro4 = filter4 ;
	filtro5 = filter5 ;
	filtro6 = filter6 ;
	filtro7 = filter7 ;

	tabName = "SYD" + d2ds( taskid(), 5 ) ;

	sd_bus* bus ;
	map<string, Unit_str*> units ;

	map<string, std::chrono::steady_clock::time_point> updates ;

	rc = sd_bus_default_system( &bus ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016A", rc ) ;
		return ;
	}

	showSystemdInfo_listUnits( bus, units ) ;
	showSystemdInfo_listUnitFiles( bus, units ) ;
	showSystemdInfo_buildTable( tabName, units, filter1, filter2, filter3, filter4, filter5, filter6, filter7 ) ;

	while ( true )
	{
		if ( updates.size() > 0 && ztdsels < 2 )
		{
			showSystemdInfo_update( bus, units, updates ) ;
			showSystemdInfo_buildTable( tabName, units, filter1, filter2, filter3, filter4, filter5, filter6, filter7 ) ;
		}
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = ( ppos == 0 ) ? "PSUT00Y1" :
				( ppos == 1 ) ? "PSUT00Y2" :
				( ppos == 2 ) ? "PSUT00Y3" : "PSUT00Y4" ;
		}
		control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		control( "PASSTHRU", "LRSCROLL", "PASOFF" ) ;
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" && ppos > 0 )
		{
			--ppos ;
			continue ;
		}
		else if ( zverb == "RIGHT" && ppos < 3 )
		{
			++ppos ;
			continue ;
		}
		if ( zcmd == "SESSIONS" )
		{
			control( "DISPLAY", "SAVE" ) ;
			showSystemdInfo_Sessions( bus ) ;
			control( "DISPLAY", "RESTORE" ) ;
			zcmd = "" ;
			continue ;
		}
		if ( zcmd == "TIMERS" )
		{
			control( "DISPLAY", "SAVE" ) ;
			showSystemdInfo_Timers( bus, units ) ;
			control( "DISPLAY", "RESTORE" ) ;
			zcmd = "" ;
			continue ;
		}
		if ( zcmd == "/" )
		{
			addpop( "", 2, 5 ) ;
			display( "PSUT00Y8" ) ;
			if ( RC == 0 )
			{
				control( "NONDISPL", "ENTER" ) ;
			}
			rempop() ;
			continue ;
		}
		if ( zcmd == "REFRESH" )
		{
			for ( auto u : units )
			{
				delete u.second ;
			}
			units.clear() ;
			sd_bus_unref( bus ) ;
			rc = sd_bus_default_system( &bus ) ;
			if ( rc < 0 )
			{
				display_error( "PSUT016A", rc ) ;
				break ;
			}
			showSystemdInfo_listUnits( bus, units ) ;
			showSystemdInfo_listUnitFiles( bus, units ) ;
			showSystemdInfo_buildTable( tabName, units, filter1, filter2, filter3, filter4, filter5, filter6, filter7 ) ;
			zcmd    = "" ;
			ztdsels = 0 ;
			continue ;
		}
		if ( zcmd == "RESET" )
		{
			ztdsels = 0  ;
			ztdtop  = 0  ;
			zcmd    = "" ;
		}
		else if ( zcmd == "X" )
		{
			addpop( "", 2, 5 ) ;
			display( "PSUT00Y7" ) ;
			if ( RC == 0 )
			{
				select( "PGM(PCMD0A) PARM(systemctl "+ parms +" ) SUSPEND" ) ;
			}
			rempop() ;
		}
		if ( filter1 != filtro1 || filter2 != filtro2 || filter3 != filtro3 || filter4 != filtro4 || filter5 != filtro5 || filter6 != filtro6 || filter7 != filtro7 )
		{
			showSystemdInfo_buildTable( tabName, units, filter1, filter2, filter3, filter4, filter5, filter6, filter7 ) ;
			filtro1 = filter1 ;
			filtro2 = filter2 ;
			filtro3 = filter3 ;
			filtro4 = filter4 ;
			filtro5 = filter5 ;
			filtro6 = filter6 ;
			filtro7 = filter7 ;
			continue ;
		}
		if ( sel == "/" )
		{
			addpop( "", 2, 5 ) ;
			display( "PSUT00Y5" ) ;
			if ( RC == 8 )
			{
				rempop() ;
				continue ;
			}
			rempop() ;
		}
		if ( sel == "D" )
		{
			select( "PGM(PCMD0A) PARM(systemctl list-dependencies -- "+ unit +" ) SUSPEND" ) ;
		}
		else if ( sel == "I" )
		{
			select( "PGM(PCMD0A) PARM(systemctl status -- "+ unit +" ) SUSPEND" ) ;
		}
		else if ( sel == "J" )
		{
			select( "PGM(PCMD0A) PARM(journalctl --unit "+ unit +" ) SUSPEND" ) ;
		}
		else if ( sel == "L" )
		{
			select( "PGM(PCMD0A) PARM(journalctl -b0 --unit "+ unit +" ) SUSPEND" ) ;
		}
		else if ( sel == "X" )
		{
			addpop( "", 2, 5 ) ;
			display( "PSUT00Y6" ) ;
			if ( RC == 0 )
			{
				select( "PGM(PCMD0A) PARM(systemctl "+ parms + " -- " + unit +" ) SUSPEND" ) ;
				updates[ unit ] = std::chrono::steady_clock::now() ;
			}
			rempop() ;
		}
		else if ( sel == "M" )
		{
			updates[ unit ] = std::chrono::steady_clock::now() ;
		}
		else if ( sel == "E" || sel == "B" )
		{
			auto it = units.find( unit ) ;
			if ( it->second->unitPath != "" )
			{
				try
				{
					if ( exists( it->second->unitPath ) )
					{
						( sel == "E" ) ? edit( it->second->unitPath ) : browse( it->second->unitPath ) ;
					}
					else
					{
						display_error( "PSUT016C", it->second->unitPath ) ;
					}
				}
				catch (...)
				{
					display_error( "PSUT016D", it->second->unitPath ) ;
				}
			}
		}
		else if ( sel == "A" || sel == "R" )
		{
			auto it = units.find( unit ) ;
			if ( it->second->unitPath != "" )
			{
				try
				{
					if ( sel == "A" )
					{
						select( "PGM(PCMD0A) PARM(--NOBROWSE systemctl enable -- " + unit +" )" ) ;
						updates[ unit ] = std::chrono::steady_clock::now() ;
					}
					else
					{
						select( "PGM(PCMD0A) PARM(--NOBROWSE systemctl disable -- " + unit +" )" ) ;
						updates[ unit ] = std::chrono::steady_clock::now() ;
					}
				}
				catch (...)
				{
					display_error( "PSUT016D", it->second->unitPath ) ;
				}
			}
		}
		else if ( sel == "S" || sel == "P" )
		{
			try
			{
				applyUnitChange( bus, unit.c_str(), ( sel == "S" ) ? "StartUnit" : "StopUnit" ) ;
				updates[ unit ] = std::chrono::steady_clock::now() ;
			}
			catch ( std::string &err )
			{
				llog( "E", err ) ;
			}
		}
		zcmd = "" ;
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2, vlist3 ) ;

	for ( auto u : units )
	{
		delete u.second ;
	}

	sd_bus_unref( bus ) ;
}


void psysutl::showSystemdInfo_listUnits( sd_bus* bus,
					 map<string, Unit_str*>& units )
{
	//
	// List systemd units and add to the units map.
	//

	int rc ;

	Unit_cc unit ;

	sd_bus_message* bMessage = nullptr ;
	sd_bus_message* reply    = nullptr ;
	sd_bus_error error       = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error, &bMessage, &reply )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( bMessage ) ;
		sd_bus_message_unref( reply ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_message_new_method_call( bus,
					     &bMessage,
					     "org.freedesktop.systemd1",
					     "/org/freedesktop/systemd1",
					     "org.freedesktop.systemd1.Manager",
					     "ListUnits" ) ;

	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_new_method_call", 10, rc ) ;
		return ;
	}

	rc = sd_bus_call( bus, bMessage, 0, &error, &reply ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call", 20, rc ) ;
		return ;
	}

	rc = sd_bus_message_enter_container( reply, SD_BUS_TYPE_ARRAY, "(ssssssouso)" ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_enter_container", 30, rc ) ;
		return ;
	}

	while ( ( rc = bus_parse_unit_info( reply, &unit ) ) > 0 )
	{
		Unit_str *u = new Unit_str( &unit ) ;
		u->set_state( get_state( unit.id, bus ) ) ;
		units[ unit.id ] = u ;
	}

	rc = sd_bus_message_exit_container( reply ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_exit_container", 40, rc ) ;
		return ;
	}
}


void psysutl::showSystemdInfo_listUnitFiles( sd_bus* bus,
					     map<string, Unit_str*>& units )
{
	//
	// List unit files and add to the units map.
	//

	int rc ;

	string uid ;
	string p ;

	Unit_cc unit ;

	const char *state = nullptr ;
	char *path = nullptr ;

	sd_bus_message* bMessage = nullptr ;
	sd_bus_message* reply      = nullptr ;
	sd_bus_error error         = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error, &bMessage, &reply )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( bMessage ) ;
		sd_bus_message_unref( reply ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_message_new_method_call( bus,
					     &bMessage,
					     "org.freedesktop.systemd1",
					     "/org/freedesktop/systemd1",
					     "org.freedesktop.systemd1.Manager",
					     "ListUnitFiles" ) ;

	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_new_method_call", 50, rc ) ;
		return ;
	}

	rc = sd_bus_call( bus, bMessage, 0, &error, &reply ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call", 60, rc ) ;
		return ;
	}

	rc = sd_bus_message_enter_container( reply, SD_BUS_TYPE_ARRAY, "(ss)" ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_enter_container_call", 70, rc ) ;
		return ;
	}

	while ( ( rc = sd_bus_message_read( reply, "(ss)", &path, &state ) ) > 0)
	{
		p   = string( path ) ;
		uid = p.substr( p.find_last_of( '/' ) + 1 ) ;
		auto it = units.find( uid ) ;
		if ( it == units.end() )
		{
			Unit_str *u  = new Unit_str() ;
			u->unitPath  = path ;
			u->set_state( state ) ;
			u->loadState = "unloaded" ;
			units[ uid ] = u ;
		}
		else
		{
			it->second->Path     = it->second->unitPath ;
			it->second->unitPath = path ;
			it->second->set_state( state ) ;
		}
		path  = nullptr ;
		state = nullptr ;
	}

	rc = sd_bus_message_exit_container( reply ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_exit_container_call", 80, rc ) ;
		return ;
	}
}


void psysutl::showSystemdInfo_update( sd_bus* bus,
				      map<string, Unit_str*>& units,
				      map<string, std::chrono::steady_clock::time_point>& updates )
{
	//
	// Update units with any changes to the list of units in the updates map.
	// Remove any units from the update list after 10 seconds.
	//

	set<string>removals ;

	for ( const auto& unit : updates )
	{
		if ( std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now() - unit.second ).count() > 10 )
		{
			removals.insert( unit.first ) ;
		}
		showSystemdInfo_listUnitsByPatterns( bus, units, unit.first ) ;
	}

	for ( const auto& unit : removals )
	{
		updates.erase( unit ) ;
	}
}


void psysutl::showSystemdInfo_listUnitsByPatterns( sd_bus* bus,
						   map<string, Unit_str*>& units,
						   const string& name )
{
	//
	// List unit by name and update/add to the units map.
	// Name is not a generic so only one value expected to be returned.
	// If not found, get the unit file and replace in the units map.
	//

	int rc ;

	Unit_cc unit ;

	char* pat   = new char[ name.length() + 1 ] ;

	char** arg_states = nullptr ;
	char** patterns   = &pat ;

	strcpy( pat, name.c_str() ) ;

	sd_bus_message* bMessage = nullptr ;
	sd_bus_message* reply    = nullptr ;
	sd_bus_error error       = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error, &bMessage, &reply, &pat )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( bMessage ) ;
		sd_bus_message_unref( reply ) ;
		delete [] pat ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_message_new_method_call( bus,
					     &bMessage,
					     "org.freedesktop.systemd1",
					     "/org/freedesktop/systemd1",
					     "org.freedesktop.systemd1.Manager",
					     "ListUnitsByPatterns" ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_new_method_call", 90, rc ) ;
		return ;
	}

	rc = sd_bus_message_append_strv( bMessage, arg_states ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_append_strv", 100, rc ) ;
		return ;
	}

	rc = sd_bus_message_append_strv( bMessage, patterns ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_append_strv", 110, rc ) ;
		return ;
	}

	rc = sd_bus_call( bus, bMessage, 0, &error, &reply ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call", 120, rc ) ;
		return ;
	}

	rc = sd_bus_message_enter_container( reply, SD_BUS_TYPE_ARRAY, "(ssssssouso)") ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_enter_container_call", 130, rc ) ;
		return ;
	}

	rc = bus_parse_unit_info( reply, &unit ) ;
	if ( rc > 0 )
	{
		auto it = units.find( name ) ;
		if ( it == units.end() )
		{
			Unit_str *u = new Unit_str( &unit ) ;
			u->set_state( get_state( name, bus ) ) ;
			units[ unit.id ] = u ;
		}
		else
		{
			*it->second = Unit_str( &unit ) ;
			it->second->Path = unit.unitPath ;
			it->second->set_state( get_state( name, bus ) ) ;
		}
	}
	else if ( rc == 0 )
	{
		showSystemdInfo_listUnitFilesByPatterns( bus, units, name ) ;
	}
	else
	{
		display_error( "PSUT016B", "sd_bus_parse_unit_info", 140, rc ) ;
		return ;
	}

	rc = sd_bus_message_exit_container(reply) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_exit_container_call", 150, rc ) ;
		return ;
	}
}


void psysutl::showSystemdInfo_listUnitFilesByPatterns( sd_bus* bus,
						       map<string, Unit_str*>& units,
						       const string& name )
{
	//
	// List unit file by name and replace/add to the units map.
	// Name is not a generic so only one value expected to be returned.
	//

	int rc ;

	Unit_cc unit ;

	char** arg_states = nullptr ;

	char* state = nullptr ;
	char* path  = nullptr ;
	char* pat   = new char[ name.length() + 1 ] ;

	char** patterns = &pat ;

	strcpy( pat, name.c_str() ) ;

	sd_bus_message* bMessage = nullptr ;
	sd_bus_message* reply    = nullptr ;
	sd_bus_error error       = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error, &bMessage, &reply, &pat )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( bMessage ) ;
		sd_bus_message_unref( reply ) ;
		delete [] pat ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_message_new_method_call( bus,
					     &bMessage,
					     "org.freedesktop.systemd1",
					     "/org/freedesktop/systemd1",
					     "org.freedesktop.systemd1.Manager",
					     "ListUnitFilesByPatterns" ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_new_method_call", 160, rc ) ;
		return ;
	}

	rc = sd_bus_message_append_strv( bMessage, arg_states ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_append_strv", 170, rc ) ;
		return ;
	}

	rc = sd_bus_message_append_strv( bMessage, patterns ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_append_strv", 180, rc ) ;
		return ;
	}

	rc = sd_bus_call( bus, bMessage, 0, &error, &reply ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call", 190, rc ) ;
		return ;
	}

	rc = sd_bus_message_enter_container( reply, SD_BUS_TYPE_ARRAY, "(ss)") ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_enter_container_call", 200, rc ) ;
		return ;
	}

	rc = sd_bus_message_read( reply, "(ss)", &path, &state) ;
	if ( rc > 0 )
	{
		Unit_str *u  = new Unit_str() ;
		u->unitPath  = path ;
		u->set_state( state ) ;
		u->loadState = "unloaded" ;
		auto it = units.find( name ) ;
		if ( it != units.end() )
		{
			delete it->second ;
		}
		units[ name ] = u ;
	}
	else if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_parse_unit_info", 210, rc ) ;
		return ;
	}

	rc = sd_bus_message_exit_container( reply ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_exit_container_call", 220, rc ) ;
		return ;
	}
}


void psysutl::applyUnitChange( sd_bus* bus,
			       const char *name,
			       const char *method )
{
	//
	// Change the status of a unit.
	//

	int rc ;

	sd_bus_error error       = SD_BUS_ERROR_NULL ;
	sd_bus_message *bMessage = nullptr ;
	sd_bus_message *reply    = nullptr ;

	BOOST_SCOPE_EXIT( &error, &bMessage, &reply )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( bMessage ) ;
		sd_bus_message_unref( reply ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_call_method( bus,
				 "org.freedesktop.systemd1",
				 "/org/freedesktop/systemd1",
				 "org.freedesktop.systemd1.Manager",
				 method,
				 &error,
				 &reply,
				 "ss",
				 name,
				 "replace-irreversibly" ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call_method " + string( method ), 230, rc ) ;
		return ;
	}

	sd_bus_error_free( &error ) ;
	sd_bus_message_unref( bMessage ) ;
}


int psysutl::bus_parse_unit_info( sd_bus_message* message,
				  Unit_cc* unit )
{
	//
	// Parse the message.
	//

	return sd_bus_message_read( message,
				    "(ssssssouso)",
				    &unit->id,
				    &unit->description,
				    &unit->loadState,
				    &unit->activeState,
				    &unit->subState,
				    nullptr,
				    &unit->unitPath,
				    nullptr,
				    nullptr,
				    nullptr ) ;
}


string psysutl::get_state( string name,
			   sd_bus* bus )
{
	//
	// Get unit file state for unit 'name'.
	//

	int rc ;

	const char* state = nullptr ;

	sd_bus_message *bMessage = nullptr ;
	sd_bus_error error       = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error, &bMessage )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( bMessage ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_call_method( bus,
				 "org.freedesktop.systemd1",
				 "/org/freedesktop/systemd1",
				 "org.freedesktop.systemd1.Manager",
				 "GetUnitFileState",
				 &error,
				 &bMessage,
				 "s",
				 name.c_str() ) ;
	if ( rc < 0 )
	{
		if ( rc != -ENOENT )
		{
			display_error( "PSUT016B", "sd_bus_call_method GetUnitFileState", 290, rc ) ;
		}
		return "" ;
	}

	rc = sd_bus_message_read( bMessage, "s", &state ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_read", 300, rc ) ;
		return "" ;
	}

	return state ?: "" ;
}


void psysutl::showSystemdInfo_buildTable( const string& tabName,
					  map<string, Unit_str*>& units,
					  string filter1,
					  string filter2,
					  string filter3,
					  const string& filter4,
					  const string& filter5,
					  const string& filter6,
					  const string& filter7 )
{
	//
	// Add the units to the lspf table applying filters.
	//
	//  filter1 -> loadState
	//  filter2 -> activeState
	//  filter3 -> subState
	//  filter4 -> description
	//  filter5 -> unit
	//  filter6 -> unit path
	//  filter7 -> path
	//
	//  Note:
	//    All matches are case insensitive.
	//    For loadState, activeState and subState, the match is from the start of the string.
	//    For all others, a regex match anywhere is done with character '?' representing a
	//    single character and '*' any number of characters including 0.  Character matches
	//    include blanks.
	//

	regex expr4 ;
	regex expr5 ;
	regex expr6 ;
	regex expr7 ;

	string f4 = ( filter4 != "" ) ? "*" + filter4 + "*" : "" ;
	string f5 = ( filter5 != "" ) ? "*" + filter5 + "*" : "" ;
	string f6 = ( filter6 != "" ) ? "*" + filter6 + "*" : "" ;
	string f7 = ( filter7 != "" ) ? "*" + filter7 + "*" : "" ;

	if ( filter4 != "" )
	{
		try
		{
			expr4.assign( conv_regex_any( f4, '?', '*' ), boost::regex_constants::icase ) ;
		}
		catch  ( boost::regex_error& e )
		{
			f4 = "" ;
		}
	}
	if ( filter5 != "" )
	{
		try
		{
			expr5.assign( conv_regex_any( f5, '?', '*' ), boost::regex_constants::icase ) ;
		}
		catch  ( boost::regex_error& e )
		{
			f5 = "" ;
		}
	}
	if ( filter6 != "" )
	{
		try
		{
			expr6.assign( conv_regex_any( f6, '?', '*' ), boost::regex_constants::icase ) ;
		}
		catch  ( boost::regex_error& e )
		{
			f6 = "" ;
		}
	}
	if ( filter7 != "" )
	{
		try
		{
			expr7.assign( conv_regex_any( f7, '?', '*' ), boost::regex_constants::icase ) ;
		}
		catch  ( boost::regex_error& e )
		{
			f7 = "" ;
		}
	}

	tbcreate( tabName,
		  "",
		  "(SEL STATE LSTATE ASTATE SSTATE DESCR UNIT UUNIT PATH1 PATH2 STATEX)",
		  NOWRITE,
		  REPLACE ) ;

	for ( auto& u : units )
	{
		if ( ( !abbrev( upper( u.second->loadState ), upper( filter1 ) ) ) ||
		     ( !abbrev( upper( u.second->activeState ), upper( filter2 ) ) ) ||
		     ( !abbrev( upper( u.second->subState ), upper( filter3 ) ) ) ||
		     ( f4 != "" && !regex_match( u.second->description.begin(), u.second->description.end(), expr4 ) ) ||
		     ( f5 != "" && !regex_match( u.first.begin(), u.first.end(), expr5 ) ) ||
		     ( f6 != "" && !regex_match( u.second->unitPath.begin(), u.second->unitPath.end(), expr6 ) ) ||
		     ( f7 != "" && !regex_match( u.second->Path.begin(), u.second->Path.end(), expr7 ) ) )
		{
			continue ;
		}
		tbvclear( tabName ) ;
		vreplace( "LSTATE", u.second->loadState ) ;
		vreplace( "ASTATE", ( u.second->activeState == "" ) ? "-" : u.second->activeState ) ;
		vreplace( "SSTATE", ( u.second->subState == "" ) ? "-" : u.second->subState ) ;
		vreplace( "DESCR", ( u.second->description == "" ) ? "-" : u.second->description ) ;
		vreplace( "PATH1", u.second->unitPath ) ;
		vreplace( "PATH2", u.second->Path ) ;
		vreplace( "UNIT", u.first ) ;
		vreplace( "UUNIT", upper( u.first ) ) ;
		vreplace( "STATEX", u.second->state_str ) ;
		vreplace( "STATE", u.second->get_state() ) ;
		tbadd( tabName ) ;
	}

	tbsort( tabName, "(UUNIT,C,A)" ) ;
}


void psysutl::showSystemdInfo_Sessions( sd_bus* bus )
{
	//
	// List sessions.
	//

	string zcmd ;
	string zverb ;
	string panel ;
	string cursor ;
	string tabName ;

	string sel ;
	string user ;
	string session ;

	tabName = "SYS" + d2ds( taskid(), 5 ) ;

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD ZVERB SEL USER SESS" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &zverb, &sel, &user, &session ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			showSystemdInfo_SessionsBuildTable( tabName, bus ) ;
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00YB" ;
		}
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "/" )
		{
			addpop( "", 2, 5 ) ;
			display( "PSUT00YD" ) ;
			if ( RC == 8 )
			{
				rempop() ;
				continue ;
			}
			rempop() ;
		}
		if ( sel == "I" )
		{
			select( "PGM(PCMD0A) PARM(loginctl user-status "+ user +") SUSPEND" ) ;
		}
		else if ( sel == "L" )
		{
			showSystemdInfo_SessionsAction( bus, "LockSession", session ) ;
		}
		else if ( sel == "U" )
		{
			showSystemdInfo_SessionsAction( bus, "UnlockSession", session ) ;
		}
		else if ( sel == "T" )
		{
			showSystemdInfo_SessionsAction( bus, "TerminateSession", session ) ;
		}
		else if ( sel == "A" )
		{
			showSystemdInfo_SessionsAction( bus, "ActivateSession", session ) ;
		}
	}

	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showSystemdInfo_SessionsBuildTable( const string& tabName,
						  sd_bus* bus )
{
	//
	// For each active timer, get details and add the timer to the lspf table.
	//

	tbcreate( tabName,
		  "",
		  "(SEL SESS UID USER SEAT TTY STATE PATH)",
		  NOWRITE,
		  REPLACE ) ;

	showSystemdInfo_SessionsDetails( bus,
					 tabName ) ;

	tbsort( tabName, "(SESS,N,A)" ) ;
}


void psysutl::showSystemdInfo_SessionsDetails( sd_bus* bus,
					       const string& tabName )
{
	//
	// Get session details.
	//

	int rc ;

	string path ;
	string state ;

	sd_bus_message* reply = nullptr ;
	sd_bus_error error    = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error, &reply )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( reply ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_call_method( bus,
				 "org.freedesktop.login1",
				 "/org/freedesktop/login1",
				 "org.freedesktop.login1.Manager",
				 "ListSessions",
				 &error,
				 &reply,
				 nullptr ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call_method", 310, rc ) ;
		return  ;
	}

	rc = sd_bus_message_enter_container( reply, 'a', "(susso)" ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_enter_container", 320, rc ) ;
		return  ;
	}

	while ( true )
	{
		uint32_t uid;
		const char *id, *user, *seat, *object, *tty = nullptr ;
		rc = sd_bus_message_read(reply, "(susso)", &id, &uid, &user, &seat, &object);
		if ( rc < 0 )
		{
			display_error( "PSUT016B", "sd_bus_message_read", 330, rc ) ;
			return  ;
		}
		if ( rc == 0 ) { break ; }

		sd_bus_message* reply_tty  = nullptr ;
		sd_bus_error error_tty     = SD_BUS_ERROR_NULL ;
		rc = sd_bus_get_property( bus,
					  "org.freedesktop.login1",
					  object,
					  "org.freedesktop.login1.Session",
					  "TTY",
					  &error_tty,
					  &reply_tty,
					  "s" ) ;
		if ( rc < 0 )
		{
			display_error( "PSUT016B", "sd_bus_get_property", 340, rc ) ;
			return  ;
		}
		rc = sd_bus_message_read( reply_tty, "s", &tty ) ;
		if ( rc < 0 )
		{
			display_error( "PSUT016B", "sd_bus_message_read", 350, rc ) ;
			return  ;
		}

		tbvclear( tabName ) ;
		vreplace( "SESS", id ) ;
		vreplace( "UID", d2ds( uid ) ) ;
		vreplace( "USER", user ) ;
		vreplace( "SEAT", seat ) ;
		vreplace( "TTY", ( strip( tty ) == "" ) ? "-" : tty ) ;
		path = get_session_path( bus, id ) ;
		get_session_info( bus, path, state ) ;
		vreplace( "PATH", path ) ;
		vreplace( "STATE", state ) ;
		tbadd( tabName ) ;
	}
}


void psysutl::showSystemdInfo_SessionsAction( sd_bus* bus,
					      const string& action,
					      const string& session )
{
	//
	// Issue action against a session.
	//

	int rc ;

	sd_bus_error error = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error )
	{
		sd_bus_error_free( &error ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_call_method( bus,
				 "org.freedesktop.login1",
				 "/org/freedesktop/login1",
				 "org.freedesktop.login1.Manager",
				 action.c_str(),
				 &error,
				 nullptr,
				 "s",
				 session.c_str() ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call_method", 360, rc ) ;
		return  ;
	}
}


string psysutl::get_session_path( sd_bus* bus,
				  const char* session_id )
{
	//
	// Get the session path.
	//

	sd_bus_message* reply  = nullptr ;
	sd_bus_error error     = SD_BUS_ERROR_NULL ;

	int rc ;

	char* ans = nullptr ;

	BOOST_SCOPE_EXIT( &error, &reply )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( reply ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_call_method( bus,
				 "org.freedesktop.login1",
				 "/org/freedesktop/login1",
				 "org.freedesktop.login1.Manager",
				 "GetSession",
				 &error,
				 &reply,
				 "s",
				 session_id ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_call_method", 370, rc ) ;
		return "" ;
	}

	rc = sd_bus_message_read( reply, "o", &ans ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_message_read", 380, rc ) ;
		return "" ;
	}

	return ( ans ) ?: "" ;
}


void psysutl::get_session_info( sd_bus* bus,
				const string& path,
				string& state )
{
	//
	// Get session info for a path.
	//

	sd_bus_message* reply  = nullptr ;
	sd_bus_error error     = SD_BUS_ERROR_NULL ;

	int rc ;

	char* val = nullptr ;

	state = "" ;

	BOOST_SCOPE_EXIT( &error, &reply )
	{
		sd_bus_error_free( &error ) ;
		sd_bus_message_unref( reply ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_get_property_string( bus,
					 "org.freedesktop.login1",
					 path.c_str(),
					 "org.freedesktop.login1.Session",
					 "State",
					 &error,
					 &val ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_get_property_trivial", 390, rc ) ;
		return  ;
	}

	state = string( val ) ;
}


void psysutl::showSystemdInfo_Timers( sd_bus* bus,
				      map<string, Unit_str*>& units )
{
	//
	// List systemd timer units plus details.
	//

	string zcmd ;
	string zverb ;
	string panel ;
	string cursor ;
	string tabName ;

	string sel ;
	string unit ;
	string trigger ;

	tabName = "SYT" + d2ds( taskid(), 5 ) ;

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;
	int ppos    = 0 ;

	int crp ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD ZVERB SEL UNIT TRIGGER" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &zverb, &sel, &unit, &trigger ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			showSystemdInfo_TimersBuildTable( tabName, bus, units ) ;
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = ( ppos == 0 ) ? "PSUT00Y9" : "PSUT00YA" ;
		}
		control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		control( "PASSTHRU", "LRSCROLL", "PASOFF" ) ;
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" && ppos > 0 )
		{
			--ppos ;
			continue ;
		}
		else if ( zverb == "RIGHT" && ppos < 1 )
		{
			++ppos ;
			continue ;
		}
		if ( sel == "/" )
		{
			addpop( "", 2, 5 ) ;
			display( "PSUT00YC" ) ;
			if ( RC == 8 )
			{
				rempop() ;
				continue ;
			}
			rempop() ;
		}
		if ( sel == "D" )
		{
			select( "PGM(PCMD0A) PARM(systemctl list-dependencies -- "+ unit +" ) SUSPEND" ) ;
		}
		else if ( sel == "I" )
		{
			select( "PGM(PCMD0A) PARM(systemctl status -- "+ unit +" "+ trigger +") SUSPEND" ) ;
		}
		else if ( sel == "J" )
		{
			select( "PGM(PCMD0A) PARM(journalctl --unit "+ unit +" ) SUSPEND" ) ;
		}
		else if ( sel == "L" )
		{
			select( "PGM(PCMD0A) PARM(journalctl -b0 --unit "+ unit +" ) SUSPEND" ) ;
		}
		else if ( sel == "E" || sel == "B" )
		{
			auto it = units.find( unit ) ;
			if ( it->second->unitPath != "" )
			{
				try
				{
					if ( exists( it->second->unitPath ) )
					{
						( sel == "E" ) ? edit( it->second->unitPath ) : browse( it->second->unitPath ) ;
					}
					else
					{
						display_error( "PSUT016C", it->second->unitPath ) ;
					}
				}
				catch (...)
				{
					display_error( "PSUT016D", it->second->unitPath ) ;
				}
			}
		}
		else if ( sel == "T" || sel == "U" )
		{
			auto it = units.find( trigger ) ;
			if ( it->second->unitPath != "" )
			{
				try
				{
					if ( exists( it->second->unitPath ) )
					{
						( sel == "U" ) ? edit( it->second->unitPath ) : browse( it->second->unitPath ) ;
					}
					else
					{
						display_error( "PSUT016C", it->second->unitPath ) ;
					}
				}
				catch (...)
				{
					display_error( "PSUT016D", it->second->unitPath ) ;
				}
			}
		}
	}

	vdelete( vlist1, vlist2 ) ;
}


void psysutl::showSystemdInfo_TimersBuildTable( const string& tabName,
						sd_bus* bus,
						map<string, Unit_str*>& units )
{
	//
	// For each active timer, get details and add the timer to the lspf table.
	//

	string tleft ;
	string tlefts ;
	string tnext ;
	string tlast ;
	string tpassed ;
	string triggered ;

	tbcreate( tabName,
		  "",
		  "(SEL UNIT NEXT DESCR LEFT LEFTS LAST PASSED TRIGGER)",
		  NOWRITE,
		  REPLACE ) ;

	for ( const auto& u : units )
	{
		if ( suffix( u.first, ".timer" ) && u.second->activeState == "active" )
		{
			tbvclear( tabName ) ;
			showSystemdInfo_TimerDetails( bus,
						      u.second->Path,
						      tleft,
						      tlefts,
						      tnext,
						      tpassed,
						      tlast,
						      triggered ) ;
			vreplace( "UNIT", u.first ) ;
			vreplace( "LEFT", tleft ) ;
			vreplace( "NEXT", tnext ) ;
			vreplace( "LAST", tlast ) ;
			vreplace( "PASSED", tpassed ) ;
			vreplace( "DESCR", ( u.second->description == "" ) ? "-" : u.second->description ) ;
			vreplace( "TRIGGER", triggered ) ;
			vreplace( "LEFTS", tlefts ) ;
			tbadd( tabName ) ;
		}
	}

	tbsort( tabName, "(LEFTS,N,A)" ) ;
}


void psysutl::showSystemdInfo_TimerDetails( sd_bus* bus,
					    const string& path,
					    string& tleft,
					    string& tlefts,
					    string& tnext,
					    string& tpassed,
					    string& tlast,
					    string& triggered )
{
	//
	// Get timer details.
	//   o) Next run interval and time.
	//   o) Last run interval and time.
	//   o) Trigger unit.
	//

	int rc ;

	uint64_t realtime ;
	uint64_t monotonic ;
	uint64_t last ;

	uint32_t delta ;

	char buf[ 32 ] ;

	char** trigger ;
	char*** ret = &trigger ;

	time_t rawtime ;
	time_t seconds ;

	struct tm* time_info = nullptr ;

	tleft     = "-" ;
	tlefts    = "-" ;
	tnext     = "-" ;
	tpassed   = "-" ;
	tlast     = "-" ;
	triggered = "-" ;

	sd_bus_error error = SD_BUS_ERROR_NULL ;

	BOOST_SCOPE_EXIT( &error )
	{
		sd_bus_error_free( &error ) ;
	}
	BOOST_SCOPE_EXIT_END


	rc = sd_bus_get_property_trivial( bus,
					  "org.freedesktop.systemd1",
					  path.c_str(),
					  "org.freedesktop.systemd1.Timer",
					  "NextElapseUSecMonotonic",
					  &error,
					  't',
					  &monotonic ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_get_property_trivial", 400, rc ) ;
		return  ;
	}

	rc = sd_bus_get_property_trivial(
			bus,
			"org.freedesktop.systemd1",
			path.c_str(),
			"org.freedesktop.systemd1.Timer",
			"NextElapseUSecRealtime",
			&error,
			't',
			&realtime);
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_get_property_trivial", 410, rc ) ;
		return  ;
	}

	seconds = time( nullptr ) ;

	if ( monotonic == 0 )
	{
		rawtime   = realtime / 1000000 ;
		time_info = localtime( &rawtime ) ;
		if ( time_info )
		{
			strftime( buf, sizeof( buf ), "%a %Y/%m/%d %H:%M:%S GMT", time_info ) ;
			tnext  = buf ;
			delta  = realtime/1000000 - seconds ;
			tlefts = d2ds( delta ) ;
			tleft  = delta_time( delta ) ;
		}
	}
	else if ( monotonic != UINT64_MAX )
	{
		struct timespec ts ;
		if ( clock_gettime( CLOCK_MONOTONIC, &ts ) != 0 )
		{
			setmsg( "PSUT016E" ) ;
			return  ;
		}
		delta  = monotonic/1000000 - ts.tv_sec ;
		tlefts = d2ds( delta ) ;
		tleft  = delta_time( delta ) ;

		rawtime   = seconds + delta ;
		time_info = localtime( &rawtime ) ;
		if ( time_info )
		{
			strftime( buf, sizeof( buf ), "%a %Y/%m/%d %H:%M:%S GMT", time_info ) ;
			tnext = buf ;
		}
	}

	rc = sd_bus_get_property_trivial( bus,
					  "org.freedesktop.systemd1",
					  path.c_str(),
					  "org.freedesktop.systemd1.Timer",
					  "LastTriggerUSec",
					  &error,
					  't',
					  &last ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_get_property_trivial", 420, rc ) ;
		return  ;
	}

	if ( last > 0 )
	{
		rawtime   = last / 1000000 ;
		time_info = localtime( &rawtime ) ;
		if ( time_info )
		{
			strftime( buf, sizeof( buf ), "%a %Y/%m/%d %H:%M:%S GMT", time_info ) ;
			tlast   = buf ;
			delta   = seconds - last/1000000 ;
			tpassed = delta_time( delta ) ;
		}
	}

	rc = sd_bus_get_property_strv( bus,
				       "org.freedesktop.systemd1",
				       path.c_str(),
				       "org.freedesktop.systemd1.Unit",
				       "Triggers",
				       &error,
				       ret ) ;
	if ( rc < 0 )
	{
		display_error( "PSUT016B", "sd_bus_get_property_strv", 430, rc ) ;
		return  ;
	}

	triggered = string( *trigger ) ;
}


/**************************************************************************************************************/
/**********************************          JOURNAL DISPLAY                ***********************************/
/**************************************************************************************************************/


void psysutl::showJournal()
{
	//
	// Simple utility to show selectable journal boot entries.
	//

	string tabName ;
	string panel ;
	string cursor ;

	string sel ;
	string zcmd ;
	string bootid ;

	string jfield1 ;
	string jvalue1 ;
	string filters ;

	int ztdtop  = 1 ;
	int ztdsels = 0 ;
	int csrrow  = 0 ;

	int crp ;

	const string vlist1 = "ZTDTOP ZTDSELS CRP" ;
	const string vlist2 = "ZCMD SEL BOOTID" ;
	const string vlist3 = "JFIELD1" ;
	const string vlist4 = "JVALUE1" ;

	vdefine( vlist1, &ztdtop, &ztdsels, &crp ) ;
	vdefine( vlist2, &zcmd, &sel, &bootid ) ;
	vdefine( vlist3, &jfield1 ) ;
	vdefine( vlist4, &jvalue1 ) ;

	tabName = "JRN" + d2ds( taskid(), 5 ) ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			if ( !showJournal_buildTable( tabName ) )
			{
				return ;
			}
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PSUT00J1" ;
		}
		tbdispl( tabName,
			 panel,
			 "",
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC > 4 ) { break ; }
		if ( zcmd == "FILTER" )
		{
			showJournal_setFilter() ;
			zcmd = "" ;
			continue ;
		}
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "S" || sel == "B" )
		{
			select( "PGM(PCMD0A) PARM(journalctl --b "+ bootid +" ) SUSPEND" ) ;
		}
		else if ( sel == "F" )
		{
			filters = "" ;
			vget( vlist3, PROFILE ) ;
			vget( vlist4, PROFILE ) ;
			if ( jfield1 != "" && jvalue1 != "" )
			{
				filters = jfield1 + "='" + jvalue1 + "'" ;
			}
			select( "PGM(PCMD0A) PARM(journalctl "+filters+" --b "+ bootid +" ) SUSPEND" ) ;
		}
	}

	tbend( tabName ) ;

	vdelete( vlist1, vlist2, vlist3, vlist4 ) ;
}


void psysutl::showJournal_setFilter()
{
	//
	// Set the filters for the journalctl command.
	//

	int rc ;
	int zcurpos ;

	vector<string> results ;

	string temp ;
	string zcmd ;
	string zcurfld ;

	string jfield1 ;
	string jvalue1 ;

	const string vlist1 = "ZCMD ZCURFLD" ;
	const string vlist2 = "JFIELD1" ;
	const string vlist3 = "JVALUE1" ;
	const string vlist4 = "ZCURPOS" ;

	vdefine( vlist1, &zcmd, &zcurfld ) ;
	vdefine( vlist2, &jfield1 ) ;
	vdefine( vlist3, &jvalue1 ) ;
	vdefine( vlist4, &zcurpos ) ;

	vget( vlist2, PROFILE ) ;
	vget( vlist3, PROFILE ) ;

	addpop() ;
	while ( true )
	{
		display( "PSUT00J2" ) ;
		if ( RC == 8 )
		{
			break ;
		}
		if ( zcmd == "PROMPT" )
		{
			if ( findword( zcurfld, vlist2 ) )
			{
				execute_cmd( rc, "journalctl --fields", results ) ;
				addpop( zcurfld, -2, -4 ) ;
				temp = showJournal_prompt( results, jfield1, zcurpos ) ;
				rempop() ;
				if ( temp != "" )
				{
					jfield1 = temp ;
				}
			}
			else if ( findword( zcurfld, vlist3 ) && jfield1 != "" )
			{
				execute_cmd( rc, "journalctl -F "+ jfield1, results ) ;
				addpop( zcurfld, -2, -4 ) ;
				temp = showJournal_prompt( results ) ;
				rempop() ;
				if ( temp != "" )
				{
					jvalue1 = temp ;
				}
			}
			else
			{
				setmsg( "PSYS022A" ) ;
				zcmd = "" ;
				continue ;
			}
		}
	}
	rempop() ;

	vdelete( vlist1, vlist2, vlist3, vlist4 ) ;
}


string psysutl::showJournal_prompt( const vector<string>& entries,
				    const string& jfield1,
				    int zcurpos )
{
	//
	// Diaplay a prompt list from the entries vector.
	//

	int zcurinx ;

	string entry ;
	string selentry ;
	string tabName ;

	const string vlist1 = "ENTRY" ;
	const string vlist2 = "ZCURINX" ;

	vdefine( vlist1, &entry ) ;
	vdefine( vlist2, &zcurinx ) ;

	tabName = "JRNP" + d2ds( taskid(), 4 ) ;

	tbcreate( tabName,
		  "",
		  "(ENTRY)",
		  NOWRITE ) ;

	for ( auto& e : entries )
	{
		if ( zcurpos == 1 || jfield1.size() < ( zcurpos - 1 ) || abbrev( e, jfield1.substr( 0, zcurpos-1 ) ) )
		{
			entry = e ;
			tbadd( tabName ) ;
		}
	}

	tbsort( tabName, "(ENTRY,C,A)" ) ;
	tbtop( tabName ) ;
	tbdispl( tabName,
		 "PSUT00J3",
		 "",
		 "",
		 0,
		 1,
		 "NO" ) ;
	if ( RC > 4 )
	{
		tbend( tabName ) ;
		vdelete( vlist1, vlist2 ) ;
		return "" ;
	}
	if ( zcurinx > 0 )
	{
		tbtop( tabName ) ;
		tbskip( tabName, zcurinx ) ;
		selentry = entry ;
	}

	tbend( tabName ) ;

	vdelete( vlist1, vlist2 ) ;

	return selentry ;
}


bool psysutl::showJournal_buildTable( const string& tabName )
{
	//
	// Build the boot list table from the journalctl command output.
	//

	int rc ;

	vector<string> results ;

	execute_cmd( rc, "journalctl --list-boots", results ) ;
	if ( rc != 0 )
	{
		display_error( "PSUT011F", d2ds( rc ) ) ;
		return false ;
	}

	tbcreate( tabName,
		  "",
		  "(SEL INDEX BOOTID EFIRST ELAST)",
		  NOWRITE,
		  REPLACE ) ;

	for ( auto& l : results )
	{
		if ( word( l, 1 ) == "IDX" ) { continue ; }
		tbvclear( tabName ) ;
		vreplace( "INDEX", right( word( l, 1 ), 4, ' ' ) ) ;
		vreplace( "BOOTID", word( l, 2 ) ) ;
		vreplace( "EFIRST", subword( l, 3, 4 ) ) ;
		vreplace( "ELAST", subword( l, 7 ) ) ;
		tbadd( tabName ) ;
	}

	tbsort( tabName, "(INDEX,C,A)" ) ;

	return true ;
}


/**************************************************************************************************************/
/**********************************           UTILITY FUNCTIONS             ***********************************/
/**************************************************************************************************************/


void psysutl::get_mnt_uuid_label( const string& part,
				  string& mount,
				  string& uuid,
				  string& label )
{
	//
	// Get the mount point, UUID and label of partition part.
	//

	mount = "" ;
	uuid  = get_device_info( part, _PATH_DEV_BYUUID ) ;
	label = get_device_info( part, _PATH_DEV_BYLABEL ) ;

	struct mntent* ent ;

	FILE* afile = setmntent( _PATH_PROC_MOUNTS, "r" ) ;

	if ( afile )
	{
		while ( ( ent = getmntent( afile ) ) )
		{
			if ( part == ent->mnt_fsname )
			{
				mount = ent->mnt_dir ;
				break ;
			}
		}
	}

	endmntent( afile ) ;
}


string psysutl::get_device_info( const string& fsname,
				 const string& devpath )
{
	//
	// Get device info for file system fsname.
	//
	// This is done by scanning the files in devpath and
	// matching the symlink target to fsname and returning the symlink filename.
	//

	vector<path> vt ;

	try
	{
		copy( directory_iterator( devpath ), directory_iterator(), back_inserter( vt ) ) ;
	}
	catch (...)
	{
		return "" ;
	}

	for ( auto& ent : vt )
	{
		if ( is_symlink( ent ) && fsname == canonical( read_symlink( ent ), ent.parent_path() ) )
		{
			return ent.filename().string() ;
		}
	}

	return "" ;
}


string psysutl::get_space( const string& blksize,
			   const string& n )
{
	//
	// Return the space in K/M/G/T/P given block size and number of blocks.
	// Round to 1 decimal place.
	//
	// Use size_t version of functions as these numbers can get pretty big!
	//

	return d2size( ds2d( blksize ) * ds2size_t( n ), 1 ) ;
}


string psysutl::get_pcent_space_used( const string& mount )
{
	//
	// Return the percentage space used.
	//

	string ipcent ;

	struct statvfs buf ;

	if ( mount != "" && statvfs( mount.c_str(), &buf ) == 0 && buf.f_blocks > 0 )
	{
		ipcent = to_string( ( ( ( buf.f_blocks - buf.f_bavail ) * 1000 / buf.f_blocks ) + 5 ) / 10 ) + "%" ;
	}

	return ipcent ;
}


void psysutl::get_swaps( set<string>& swaps )
{
	//
	// Store the list of swap partitions.
	//

	string line ;

	std::ifstream if_swaps( _PATH_PROC_SWAPS ) ;

	getline( if_swaps, line ) ;

	while ( getline( if_swaps, line ) )
	{
		swaps.insert( word( line, 1 ) ) ;
	}

	if_swaps.close() ;
}


string psysutl::get_processor( string& vendor )
{
	//
	// Get processor type.
	//

	int n = 0 ;

	string model ;
	string line ;

	std::ifstream if_cpuinfo( _PATH_PROC_CPUINFO ) ;

	while ( getline( if_cpuinfo, line ) )
	{
		if ( line.substr( 0, 9 ) == "processor" )
		{
			++n ;
		}
		else if ( vendor == "" && line.substr( 0, 9 ) == "vendor_id" )
		{
			vendor = strip( line.substr( line.find( ':' ) + 2 ) ) ;
		}
		else if ( model == "" && line.substr( 0, 10 ) == "model name" )
		{
			model = strip( line.substr( line.find( ':' ) + 2 ) ) ;
		}
	}

	if_cpuinfo.close() ;

	return to_string( n ) + " x " + model ;
}


string psysutl::convert_time( uint s )
{
	//
	// Convert seconds to days/hours/minues/seconds.
	//

	time_t seconds( s ) ;
	tm* p = gmtime( &seconds ) ;

	return to_string( p->tm_yday ) + ( ( p->tm_yday == 1 ) ? " day "    : " days " ) +
	       to_string( p->tm_hour ) + ( ( p->tm_hour == 1 ) ? " hour "   : " hours " ) +
	       to_string( p->tm_min  ) + ( ( p->tm_min  == 1 ) ? " minute " : " minutes " ) +
	       to_string( p->tm_sec  ) + ( ( p->tm_sec  == 1 ) ? " second " : " seconds " ) ;
}


string psysutl::ip_to_string( int family,
			      void* addr )
{
	//
	// Convert IPv4 and IPv6 from binary to text.
	//

	char buf[ INET6_ADDRSTRLEN ] ;

	inet_ntop( family,
		   addr,
		   buf,
		   sizeof( buf ) ) ;

	return buf ;
}


string psysutl::get_shared_var( const string& var )
{
	//
	// Return the dialogue variable from the SHARED pool.
	//

	string val ;

	vget( var, SHARED ) ;
	vcopy( var, val, MOVE ) ;

	return val ;
}


string psysutl::get_dialogue_var( const string& var )
{
	//
	// Return the dialogue variable.
	//

	string val ;

	vcopy( var, val, MOVE ) ;

	return val ;
}


string psysutl::add_str( const string& s1,
			 const string& s2 )
{
	//
	// Add s2 to s1, separated by a comma if s1 is not empty and
	// does not already contain the string.
	//

	return ( s1 == "" ) ? s2 :
	       ( s1.find( s2 ) != string::npos ) ? s1 : s1 + "," + s2 ;
}


void psysutl::get_property( struct udev_device* dev,
			    const char* key,
			    string& s )
{
	//
	// Set 's' to property value of key if not null.
	//

	const char* value = udev_device_get_property_value( dev, key ) ;
	if ( value )
	{
		s = value ;
	}
}


void psysutl::get_sysattr( struct udev_device* dev,
			   const char* key,
			   string& s,
			   const char* defvalue )
{
	//
	// Set 's' to property value of key if not null else set to default if specified.
	//

	const char* value = udev_device_get_sysattr_value( dev, key ) ;

	s = ( value ) ?: ( defvalue ) ?: "" ;
}


string psysutl::get_username( uint32_t uid )
{
	struct passwd* pw = getpwuid( uid ) ;

	return ( pw ) ? pw->pw_name : to_string( uid ) ;
}


string psysutl::format_kbs( float f )
{
	//
	// Format kbs into a string with two decimal places.
	//

	std::stringstream ss ;
	ss << std::fixed << std::setprecision( 2 ) << f ;

	return ss.str() + " KB/s" ;
}


string psysutl::get_program_name( const string& p )
{
	//
	// Return the program name from the full path name.
	//

	string temp = word( p, 1 ) ;

	size_t p1 = temp.find_last_of( '/' ) ;

	return ( p1 == string::npos ) ? temp : temp.substr( p1 + 1 ) ;
}


void psysutl::set_message( string& var,
			   const string& m,
			   const string& val1 )
{
	vreplace( "VAL1", val1 ) ;
	var = m ;
}


void psysutl::display_error( const string& m,
			     const string& val1 )
{
	vreplace( "VAL1", val1 ) ;
	setmsg( m ) ;
}


void psysutl::display_error( const string& m,
			     int val1 )
{
	//
	// val1 -> errno
	//

	vreplace( "VAL1", d2ds( val1 ) ) ;
	vreplace( "ERRSTR", strerror( -val1 ) ) ;
	setmsg( m ) ;
}


void psysutl::display_error( const string& m,
			     const string& val1,
			     int val2,
			     int val3 )
{
	//
	// val2 -> call number
	// val3 -> errno
	//

	vreplace( "VAL1", val1  ) ;
	vreplace( "VAL2", d2ds( val2 ) ) ;
	vreplace( "VAL3", d2ds( val3 ) ) ;
	vreplace( "ERRSTR", strerror( -val3 ) ) ;
	setmsg( m ) ;
}


string psysutl::get_mount_error( int rc )
{
	string err ;

	if ( rc & 1  ) { err  = "Incorrect invocation or permissions. " ; }
	if ( rc & 2  ) { err += "System error. " ; }
	if ( rc & 4  ) { err += "Internal mount bug. " ; }
	if ( rc & 8  ) { err += "User interrupt. " ; }
	if ( rc & 16 ) { err += "Problems writing or locking /etc/mtab. " ; }
	if ( rc & 32 ) { err += "Mount failure. " ; }
	if ( rc & 64 ) { err += "Some mounts succeeded. " ; }

	err += "(rc=" + d2ds( rc ) + "). stderr=" + stderror ;

	return err ;
}


string psysutl::get_tempname()
{
	string zuser ;
	string zscreen ;

	vcopy( "ZUSER", zuser, MOVE ) ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	boost::filesystem::path temp = boost::filesystem::temp_directory_path() /
	       boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() ;
}


int psysutl::get_user_type()
{
	//
	// Check if the user is unprivileged running in X or Wayland.
	// That way, udisksctl can ask for authorisation.
	//
	//  Return code:
	//     0  - privileged user.
	//     1  - unprivileged user under X or Wayland.
	//     2  - unprivileged user under tty.
	//

	if ( getuid() != 0 )
	{
		char* t = getenv( "XDG_SESSION_TYPE" ) ;
		if ( t && string( t ) != "tty" )
		{
			return 1 ;
		}
		else
		{
			return 2 ;
		}
	}

	return 0 ;
}


void psysutl::execute_cmd( int& rc,
			   const string& cmd,
			   vector<string>& results )
{
	//
	// Execute a command and place the output in the
	// results vector.
	//
	// Redirect STDERR/STOUT to pipes to get errors from the command.  Restore on return.
	//

	string line ;

	int fd1 ;
	int fd2 ;
	int retc ;

	int my_pipe1[ 2 ] ;
	int my_pipe2[ 2 ] ;

	char buffer[ 8192 ] ;

	results.clear() ;

	stderror = "" ;

	if ( pipe2( my_pipe1, O_NONBLOCK ) == -1 )
	{
		display_error( "PSUT015C", strerror( errno ) ) ;
		return ;
	}

	if ( pipe2( my_pipe2, O_NONBLOCK ) == -1 )
	{
		close( my_pipe1[ 0 ] ) ;
		close( my_pipe1[ 1 ] ) ;
		display_error( "PSUT015C", strerror( errno ) ) ;
		return ;
	}

	fd1 = dup( STDERR_FILENO ) ;
	dup2( my_pipe1[ 1 ], STDERR_FILENO ) ;

	fd2 = dup( STDOUT_FILENO ) ;
	dup2( my_pipe2[ 1 ], STDOUT_FILENO ) ;

	FILE* pipe { popen( cmd.c_str(), "r" ) } ;

	if ( !pipe )
	{
		rc = 3 ;
		setmsg( "PSUT011E" ) ;
		llog( "E", "POPEN failed.  Command string size="<< cmd.size() <<endl ) ;
		return ;
	}

	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		line = buffer ;
		if ( line != "" && line.back() == 0x0a )
		{
			line.pop_back() ;
		}
		results.push_back( line ) ;
	}

	rc = WEXITSTATUS( pclose( pipe ) ) ;

	fflush( stdout ) ;
	close( my_pipe2[ 0 ] ) ;
	close( my_pipe2[ 1 ] ) ;
	dup2( fd2, STDOUT_FILENO ) ;

	fflush( stderr ) ;
	close( my_pipe1[ 1 ] ) ;
	line = "" ;
	retc = read( my_pipe1[ 0 ], buffer, sizeof( buffer ) ) ;
	while ( retc != -1 )
	{
		line += string( buffer, retc ) ;
		retc = read( my_pipe1[ 0 ], buffer, sizeof( buffer ) ) ;
	}
	close( my_pipe1[ 0 ] ) ;
	dup2( fd1, STDERR_FILENO ) ;

	line = translate( line, 0x20, 0x0a ) ;
	if ( strip( line ) != "" )
	{
		stderror = line ;
		llog( "O", "STDERR " << stderror << endl ) ;
	}
}


