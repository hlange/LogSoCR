#ifndef _SCGENMOD_irqmp_
#define _SCGENMOD_irqmp_

#include "systemc.h"

struct apb_slv_in_type {
    sc_uint<16> psel;
    bool penable;
    sc_uint<32> paddr;
    bool pwrite;
    sc_uint<32> pwdata;
    sc_uint<32> pirq;
    bool testen;
    bool testrst;
    bool scanen;
    bool testoen;
};

inline ostream& operator<<(ostream& os, const apb_slv_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const apb_slv_in_type&, const std::string &) {}

inline int operator== (const apb_slv_in_type& left, const apb_slv_in_type& right) {
    return 0;
}

/************************************/

struct apb_slv_out_type {
    sc_uint<32> prdata;
    sc_uint<32> pirq;
    sc_uint<32> pconfig[2];
    int pindex;
};

inline ostream& operator<<(ostream& os, const apb_slv_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const apb_slv_out_type&, const std::string &) {}

inline int operator== (const apb_slv_out_type& left, const apb_slv_out_type& right) {
    return 0;
}

/************************************/

struct l3_irq_out_type {
    bool intack;
    sc_uint<4> irl;
    bool pwd;
    bool fpen;
};

inline ostream& operator<<(ostream& os, const l3_irq_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const l3_irq_out_type&, const std::string &) {}

inline int operator== (const l3_irq_out_type& left, const l3_irq_out_type& right) {
    return 0;
}

/************************************/

struct l3_irq_in_type {
    sc_uint<4> irl;
    bool rst;
    bool run;
    sc_uint<20> rstvec;
};

inline ostream& operator<<(ostream& os, const l3_irq_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const l3_irq_in_type&, const std::string &) {}

inline int operator== (const l3_irq_in_type& left, const l3_irq_in_type& right) {
    return 0;
}

/************************************/

template <int pindex = 0, int paddr = 0, int pmask = 4095, int ncpu = 2, int eirq = 6>
class irqmp : public sc_foreign_module
{
public:
    sc_in<bool> rst;
    sc_in<bool> clk;
    sc_in<apb_slv_in_type> apbi;
    sc_out<apb_slv_out_type> apbo;
    sc_in<l3_irq_out_type> irqi[2];
    sc_out<l3_irq_in_type> irqo[2];


    irqmp(sc_module_name nm, const char* hdl_name)
     : sc_foreign_module(nm),
       rst("rst"),
       clk("clk"),
       apbi("apbi"),
       apbo("apbo")
    {
        this->add_parameter("pindex", pindex);
        this->add_parameter("paddr", paddr);
        this->add_parameter("pmask", pmask);
        this->add_parameter("ncpu", ncpu);
        this->add_parameter("eirq", eirq);
        elaborate_foreign_module(hdl_name);
    }
    ~irqmp()
    {}

};

#endif

