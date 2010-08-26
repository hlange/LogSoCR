/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       testbench.cpp - Implementation of the                   */
/*             stimuli generator/monitor for the current testbench.    */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#include "testbench.h"

// testbench initiator thread
void testbench::initiator_thread(void) {

  // test vars
  unsigned int data;
  unsigned int tmp;
  unsigned int seta, setb, setc;
  unsigned int * debug;

  while(1) {

    debug = &tmp;

    // ************************************************************
    // * Test for cache configurations with muliple words per line
    // * ----------------------------------------------------------
    // * index  = 11  bit
    // * tag    = 16 bit
    // * offset = 5  bit
    // * sets   = 2
    // * random repl.
    // ************************************************************

    // suspend and wait for all components to be savely initialized
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // master activity
    // ===============

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 0: Read system registers (ASI 0x2) ");
    DUMP(name()," ************************************************************");

    // read/write cache control register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 1. ACTIVATE CACHES by writing the CONTROL REGISTER ");
    DUMP(name()," * (ASI 0x2 - addr 0)    ");
    DUMP(name()," ********************************************************* ");
    
    // read cache control register !
    // args: address, length, asi, flush, flushl, lock, debug 
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // [3:2] == 0b11; [1:0] = 0b11 -> dcache and icache enabled
    DUMP(name(),"cache_contr_reg: " << std::hex << data);
    assert(data==0x0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // activate caches:
    // CCR [3-2] = 11 enable dcache, CCR [1-0] enable icache
    dwrite(0x0, data |= 0xf, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 2. Read ICACHE CONFIGURATION REGISTER (ASI 0x2 - addr 8)   ");
    DUMP(name()," ********************************************************* ");    

    data=dread(0x8, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b0110 (64kb)
    // [19] lram == 0, [18:16] lsize == 0b011 (8 words per line)
    // [15:12] lramsize == 0b0000 (1kb), [11:4] lramstart == 0x8e, [3] mmu = 0
    DUMP(name(),"icache_config_reg: " << std::hex << data);
    assert(data==0x316308e0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   ");
    DUMP(name()," ********************************************************* ");    

    data=dread(0xc, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b0110 (64kb)
    // [19] lram == 0, [18:16] lsize == 0b011 (8 word per line)
    // [15:12] lramsize == 0b0000 (1kb), [11:4] lramstart == 0x8f, [3] mmu = 0
    DUMP(name(),"dcache_config_reg: " << std:: hex << data);
    assert(data==0x316308f0);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 1: Test the dcache ");
    DUMP(name()," ************************************************************");

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 4. Initialize main memory ((write miss) ");
    DUMP(name()," ************************************************************");

    // | 31 - 16 (16 bit tag) | 15 - 5 (11 bit index) | 4 - 0 (5 bit offset) |

    // 0x00010000 (tag == 1)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00010000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    // 0x00020000 (tag == 2)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00020000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    // 0x00030000 (tag == 3)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00030000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 5. Fill line 0 of one of the sets (atag 1 - read miss) ");
    DUMP(name()," ************************************************************");    
    
    for(unsigned int i = 0; i < 8; i++) {

      // args: address, length, asi, flush, flushl, lock
      data = dread((0x00010000 + (i<<2)), 4, 0x8, 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));

    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 6. Read again with index 0 and atag 1 (atag 1 - read hit) ");
    DUMP(name()," ************************************************************");        
    
    for(unsigned int i = 0; i < 8; i++) {

      data = dread((0x00010000 + (i<<2)), 4, 0x8, 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));
      assert(data==i);
    }

    unsigned int set_under_test = (*debug & 0x11);
    DUMP(name()," The data has been buffered in set: " << set_under_test);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 7. Display the cache line ");
    DUMP(name()," ************************************************************");    

    // dbg out: dcache, line 0, length 4, asi 2, ..
    dwrite(0xff, 0, 4, 2, 0, 0, 0, debug);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 8. Diagnostic checks ");
    DUMP(name()," ************************************************************");     

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read dcache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xe, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, all entries valid = 0xff
    assert(data==0x4ff);

    // read dcache entries
    for(unsigned int i = 0; i < 8; i++) {
      
      data = dread(((set_under_test << 16)|(i<<2)), 4, 0xf, 0, 0, 0, debug);
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 9. Read line with tag 2 -> fill the second set (cache miss)");
    DUMP(name()," ************************************************************");    

    for(unsigned int i = 0; i < 8; i++) {

      data = dread((0x00020000 + (i<<2)), 4, 0x8, 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 10. Invalidate set_under_test, line 0, subblock 3 ");
    DUMP(name()," * (by diagnostic write)");
    DUMP(name()," ************************************************************");    

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"
    // DATA = ATAG [26..10], LRR [9], LOCK [9], VALID [7..0]
    dwrite((set_under_test << 16), ((1 << 10)|0xf7), 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 11. Read tag 3, line 0, subblock 3  ");
    DUMP(name()," * This should give line 0 in set_under_test a new tag  ");
    DUMP(name()," * and invalidate the old data (all entries except subblock 3).");
    DUMP(name()," ************************************************************");    
    
    data = dread((0x00030000 + (3<<2)), 4, 0x8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data==3);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 12. Diagnostic check ");
    DUMP(name()," ************************************************************");     
   
    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read dcache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xe, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, subblock 3 valid = 0x08;
    assert(data==0xc08);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 13. Display the cache line ");
    DUMP(name()," ************************************************************");    

    // dbg out: dcache, line 0, length 4, asi 2, ..
    dwrite(0xff, 0, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 2: Test the icache ");
    DUMP(name()," ************************************************************");

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 14. Fill line 0 of one of the sets (atag 1 - read miss) ");
    DUMP(name()," ************************************************************");    
    
    for(unsigned int i = 0; i < 8; i++) {

      // args: address, length, flush, flushl, lock
      data = iread((0x00010000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));

    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 15. Read again with index 0 and atag 1 (atag 1 - read hit) ");
    DUMP(name()," ************************************************************");        
    
    for(unsigned int i = 0; i < 8; i++) {

      data = iread((0x00010000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));
      assert(data==i);
    }

    set_under_test = (*debug & 0x11);
    DUMP(name()," The data (instructions) has been buffered in set: " << set_under_test);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 16. Display the cache line ");
    DUMP(name()," ************************************************************");    

    // dbg out: icache, line 0, length 4, asi 2, ..
    dwrite(0xfe, 0, 4, 2, 0, 0, 0, debug);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 17. Diagnostic checks ");
    DUMP(name()," ************************************************************");     

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read icache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xc, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, all entries valid = 0xff
    assert(data==0x4ff);

    // read icache entries
    for(unsigned int i = 0; i < 8; i++) {
      
      data = dread(((set_under_test << 16)|(i<<2)), 4, 0xd, 0, 0, 0, debug);
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 18. Read line with tag 2 -> fill the second set (cache miss)");
    DUMP(name()," ************************************************************");    

    for(unsigned int i = 0; i < 8; i++) {

      data = iread((0x00020000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 19. Invalidate set_under_test, line 0, subblock 3 ");
    DUMP(name()," * (by diagnostic write)");
    DUMP(name()," ************************************************************");    

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"
    // DATA = ATAG [26..10], LRR [9], LOCK [9], VALID [7..0]
    dwrite((set_under_test << 16), ((1 << 10)|0xf7), 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 20. Read instruction cache with tag 3, line 0, subblock 3  ");
    DUMP(name()," * This should give line 0 in set_under_test a new tag  ");
    DUMP(name()," * and invalidate the old data (all entries except subblock 3).");
    DUMP(name()," ************************************************************");    
    
    data = iread((0x00030000 + (3<<2)), 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data==3);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 21. Diagnostic check ");
    DUMP(name()," ************************************************************");     
   
    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read icache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xc, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, subblock 3 valid = 0x08;
    assert(data==0xc08);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 22. Display the cache line ");
    DUMP(name()," ************************************************************");    

    // dbg out: icache, line 0, length 4, asi 2, ..
    dwrite(0xfe, 0, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 3: Test random replacement ");
    DUMP(name()," ************************************************************"); 

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 23. Load data to line 5 of both cache sets (tag 1, tag2) ");
    DUMP(name()," ************************************************************");

    // | 31 - 16 (16 bit tag) | 15 - 5 (11 bit index) | 4 - 0 (5 bit offset) |
    data = dread(0x000100a0, 4, 8, 0, 0, 0, debug);
    DUMP(this->name(),"Data: " << std::hex << data);
    assert(data==40);
    assert(CACHEREADMISS_CHECK(*debug));
    // number of set that contains the new data
    seta = (*debug & 0x3);

    data = dread(0x000200a0, 4, 8, 0, 0, 0, debug);
    assert(data==40);
    assert(CACHEREADMISS_CHECK(*debug));
    setb = (*debug & 0x3);
    // the other set shall be used to cache the data
    assert(seta != setb);
    
    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 24. Test line replacement ");
    DUMP(name()," ************************************************************");   

    // reading with tag 3 should replace seta or setb
    data = dread(0x000300a0, 4, 8, 0, 0, 0, debug);
    assert(data==40);
    assert(CACHEREADMISS_CHECK(*debug));
    setc = (*debug & 0x3);

    // reading again from the same address will produce a hit in the same set
    data = dread(0x000300a0, 4, 8, 0, 0, 0, debug);
    assert(data==40);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(setc == (*debug & 0x3));

    // read the tag which has just been replaced (bring back to cache)
    if (setc == seta) {
    
      // tag 1 was removed from cache
      data = dread(0x000100a0, 4, 8, 0, 0, 0, debug);
      assert(data==40);
      assert(CACHEREADMISS_CHECK(*debug));

      data = dread(0x000100a0, 4, 8, 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));

    } else {

      // tag 2 was removed from cache
      data = dread(0x000200a0, 4, 8, 0, 0, 0, debug);
      assert(data==40);
      assert(CACHEREADMISS_CHECK(*debug));

      data = dread(0x000200a0, 4, 8, 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));

    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 4: Test instruction burst-fetch mode ");
    DUMP(name()," ************************************************************");

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 23. Set cache control register to instr. burst fetch  ");
    DUMP(name()," ************************************************************");    
    
    // read cache control register 
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // [3:2] DCS == 0b11; [1:0] ICS = 0b11 
    DUMP(name(),"cache_contr_reg: " << std::hex << data);
    assert(data==0xf);

    // switch on bit 16 - Instruction Burst Fetch (IB)
    dwrite(0x0, data |= (1 << 16), 4, 2, 0, 0, 0, debug); 
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // check new status of cache control
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // [16] IB == 0b1, [3:2] DCS == 0b11; [1:0] ICS = 0b11 
    DUMP(name(),"cache_contr_reg: " << std::hex << data);
    assert(data==0x1000f);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 24. Invalidate instruction cache set 0  ");
    DUMP(name()," * by deleting cache tag and valid bits ");
    DUMP(name()," ************************************************************");

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"
    // DATA = ATAG [26..10], LRR [9], LOCK [9], VALID [7..0]
    dwrite(0, 0, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);
    
    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 25. Instruction read with tag 1, line 0, offset 2.  ");
    DUMP(name()," * This should cause the cache line to fill from subblock 2 ");
    DUMP(name()," * until the end (subblock 8)");
    DUMP(name()," ************************************************************");
    
    // The read from the cache line causes a read miss.
    data=iread(0x00010008, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data==2);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // If the cacheline has been properly filled by the burst
    // the following reads will be hits.
    for (unsigned int i=3; i < 8; i++) {

      data=iread((0x00010000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    wait(LOCAL_CLOCK,sc_core::SC_NS);
    sc_core::sc_stop();
  }
}

