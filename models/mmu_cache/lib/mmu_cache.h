/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu_cache.h - Class definition of a cache-subsystem.    */
/*             The cache-subsystem envelopes an instruction cache,     */
/*             a data cache and a memory management unit.              */
/*             The mmu_cache class provides two TLM slave interfaces   */
/*             for connecting the cpu to the caches and an AHB master  */
/*             interface for connection to the main memory.            */ 
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __MMU_CACHE_H__
#define __MMU_CACHE_H__

#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"

#include "amba.h"
#include "ivectorcache.h"
#include "dvectorcache.h"

#include "mmu.h"
#include "mmu_cache_if.h"

#include <math.h>
#include <ostream>

/// @brief         Top-level class of the memory sub-system for the TrapGen LEON3 simulator
/// @param         dsu
/// @icen          instruction cache enable
/// @irepl         instruction cache replacement strategy
/// @isets         number of instruction cache sets
/// @ilinesize     instruction cache line size (in bytes)
/// @isetsize      size of an instruction cache set (in kbytes)
/// @isetlock      enable instruction cache locking
/// @dcen          data cache enable
/// @drepl         data cache replacement strategy
/// @dsets         number of data cache sets
/// @dlinesize     data cache line size (in bytes)
/// @dsetsize      size of a data cache set (in kbytes)
/// @dsetlock      enable data cache locking
/// @dsnoop        enable data cache snooping
/// @ilram         enable instruction scratch pad
/// @ilramsize     size of the instruction scratch pad (in kbytes)
/// @ilramstart    start address of the instruction scratch pad
/// @dlram         enable data scratch pad
/// @dlramsize     size of the data scratch pad (in kbytes)
/// @dlramstart    start address of the data scratch pad
/// @cached
/// @mmu_en        mmu enable
/// @itlb_num      number of instruction TLBs
/// @dtlb_num      number of data TLBs
/// @tlb_type      split or shared instruction and data TLBs
/// @tlb_rep       TLB replacement strategy
/// @mmupgsz       MMU page size
template <int dsu=0, int icen=0, int irepl=0, int isets=4, int ilinesize=4, int isetsize=1, int isetlock=0,
	  int dcen=0, int drepl=0, int dsets=4, int dlinesize=4, int dsetsize=1, int dsetlock=0, int dsnoop=0,
	  int ilram=0, int ilramsize=1, int ilramstart=0x8f, int dlram=0, int dlramsize=1, int dlramstart=0x8f, int cached=0,
	  int mmu_en=0, int itlb_num=8, int dtlb_num=8, int tlb_type=1, int tlb_rep=0, int mmupgsz=0>
class mmu_cache : public mmu_cache_if, public sc_core::sc_module {

  public:

  // TLM sockets
  // -----------

  // iu3 instruction cache in/out
  tlm_utils::simple_target_socket<mmu_cache> icio;

  // iu3 data cache in/out
  tlm_utils::simple_target_socket<mmu_cache> dcio;

  // amba master socket
  amba::amba_master_socket<32> ahb_master;

  /// @brief Constructor of the top-level class of the memory sub-system (caches and mmu).
  /// @param name                               SystemC module name 
  /// @param id                                 ID of the AHB master
  /// @param icache_hit_read_response_delay     Delay on an instruction cache hit
  /// @param icache_miss_read_response_delay    Delay on an instruction cache miss
  /// @param dcache_hit_read_response_delay     Delay on a data cache read hit
  /// @param dcache_miss_read_response_delay    Delay on a data cache read miss
  /// @param dcache_write_response_delay        Delay on a data cache write (hit/miss)
  /// @param itlb_hit_response_delay            Delay on an instruction TLB hit
  /// @param itlb_miss_response_delay           Delay on an instruction TLB miss
  /// @param dtlb_hit_response_delay            Delay on a data TLB hit
  /// @param dtlb_miss_response_delay           Delay on a data TLB miss
  mmu_cache(sc_core::sc_module_name name, 
	    unsigned int id, 
	    sc_core::sc_time icache_hit_read_response_delay, 
	    sc_core::sc_time icache_miss_read_response_delay, 
	    sc_core::sc_time dcache_hit_read_response_delay, 
	    sc_core::sc_time dcache_miss_read_response_delay,
	    sc_core::sc_time dcache_write_response_delay,
	    sc_core::sc_time itlb_hit_response_delay,
	    sc_core::sc_time itlb_miss_response_delay,
	    sc_core::sc_time dtlb_hit_response_delay,
	    sc_core::sc_time dtlb_miss_response_delay);

  // member functions
  // ----------------
  // forward transport function for icio socket
  void icio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);
  // forward transport functtion for dcio socket
  void dcio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);

  // interface to AMBA master socket
  void amba_write(unsigned int addr, unsigned int * data, unsigned int length);
  void amba_read(unsigned int addr, unsigned int * data, unsigned int length);

  // read/write cache control register
  void write_ccr(unsigned int * data, sc_core::sc_time *delay);
  unsigned int read_ccr();

  // data members
  // ------------
  // instruction cache
  ivectorcache * icache;
  // data cache
  dvectorcache * dcache;
  // mmu
  mmu * srmmu;

  private:

  // CACHE CONTROL REGISTER
  // ======================
  // [1:0] instruction cache state (ICS) - indicates the current instruction cache state
  // (X0 - disabled, 01 - frozen, 11 - enabled)
  // [3:2] data cache state (DCS) - indicates the current data cache state
  // (X0 - disabled, 01 - frozen, 11 - enabled)
  // [4] instruction cache freeze on interrupt (IF) - if set the instruction cache will automatically be frozen when an asynchronous interrupt is taken
  // [5] data cache freeze on interrupt (DF) - if set the data cache will automatically be frozen when an asynchronous interrupt is taken
  // [14] - data cache flush pending (DP) - This bit is set when an data cache flush operation is in progress
  // [15] - instruction cache flush pending (IP) - This bis is set when an instruction cache flush operation is in progress
  // [16] - instruction burst fetch (IB) - This bit enables burst fill during instruction fetch
  // [21] - Flush Instruction cache (FI) - If set, will flush the instruction cache. Always reads as zero.
  // [22] - Flush data cache (FD)        - If set, will flush the data cache. Always reads as zero.
  // [23] - Data cache snoop enable (DS) - If set, will enable data cache snooping.
  unsigned int CACHE_CONTROL_REG;

  // amba related
  unsigned int master_id;
  unsigned int m_txn_count;
  unsigned int m_data_count;
  bool m_bus_granted;
  tlm::tlm_generic_payload *current_trans;
  bool m_request_pending;
  bool m_data_pending;
  bool m_bus_req_pending;
  bool m_restart_pending_req;

};

#include "mmu_cache.tpp"

#endif //__MMU_CACHE_H__
