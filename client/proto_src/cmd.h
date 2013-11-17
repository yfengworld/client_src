#ifndef CMD_H_INCLUDED
#define CMD_H_INCLUDED

/*

topology:
      
      -------------------------> login
      |                            ^
      |                            |
      |                            |-----------------
      |                                             |
   client ----------------------> gate ---------> center
                                    |               ^
                                    |               |
                                    |               |
                                    -------------> game --------------> cache
client          'c'
login           'l'
gate            'g'
center          'e'
game            'm"
cache           'a'

*/

enum
{

    /********************************* login -> center *********************************/
    LE_BEGIN = 1,
    le_user_login_request,
    LE_END,

    /********************************* center -> login *********************************/
    EL_BEGIN = 21,
    el_center_reg,
    el_user_login_reply,
    EL_END,

    /********************************* gate -> game *********************************/
    GM_BEGIN = 41,
    GM_END,

    /********************************* game -> gate *********************************/
    MG_BEGIN = 61,
    MG_END,

    /********************************* gate -> center *********************************/
    GE_BEGIN = 81,
    ge_gate_reg,
    ge_user_session_reply,
    GE_END,

    /********************************* center -> gate *********************************/
    EG_BEGIN = 101,
    eg_user_session_request,
    EG_END,

    /********************************* gate -> cache *********************************/
    GA_BEGIN = 121,
    GA_END,

    /********************************* cache -> gate *********************************/
    AG_BEGIN = 141,
    AG_END,

    /********************************* game -> center *********************************/
    ME_BEGIN = 161,
    ME_END,

    /********************************* center -> game *********************************/
    EM_BEGIN = 181,
    EM_END,

    /********************************* game -> cache *********************************/
    MA_BEGIN = 201,
    MA_END,

    /********************************* cache -> game *********************************/
    AM_BEGIN = 221,
    AM_END,

    /************************
     * external divding line
     * **********************/

    CS_BEGIN = 1000,

    /********************************* client -> login *********************************/
    CL_BEGIN = CS_BEGIN,
    cl_login_request,
    CL_END,

    /********************************* client -> gate *********************************/
    CG_BEGIN = 1011,
    cg_connect_request,
    CG_END,

    /********************************* client -> center *********************************/
    CE_BEGIN = 1021,
    CE_END,

    /********************************* client -> cache *********************************/
    CA_BEGIN = 1031,
    CA_END,

    /********************************* client -> game *********************************/
    CM_BEGIN = 1041,
    CM_END,

    CS_END = CM_END,
    SC_BEGIN = 2000,

    /********************************* login -> client *********************************/
    LC_BEGIN = SC_BEGIN,
    lc_login_reply,
    LC_END,

    /********************************* gate -> client *********************************/
    GC_BEGIN = 2011,
    gc_connect_reply,
    GC_END,

    /********************************* center -> client *********************************/
    EC_BEGIN = 2021,
    ec_test,
    EC_END,

    /********************************* cache -> client *********************************/
    AC_BEGIN = 2031,
    AC_END,

    /********************************* game -> client *********************************/
    MC_BEGIN = 2041,
    MC_END,

    SC_END = MC_END
};

int check_cmd();

typedef void (*proto_reg_func)();
typedef struct proto_reg_item proto_reg_item;
struct proto_reg_item {
    proto_reg_item *next;
    proto_reg_func func;
};

proto_reg_item *proto_reg(proto_reg_item *head, proto_reg_func func);

#endif /* CMD_H_INCLUDED */
