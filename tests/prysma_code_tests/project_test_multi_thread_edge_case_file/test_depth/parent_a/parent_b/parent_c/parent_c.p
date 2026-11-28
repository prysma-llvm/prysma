include "child_a.p";
include "child_b.p";
include "child_c.p";
include "child_d.p";
include "child_e.p";

fn void printParentC()
{
    call printChildA_C(); call backSlashN();
    call printChildB_C(); call backSlashN();
    call printChildC_C(); call backSlashN();
    call printChildD_C(); call backSlashN();
    call printChildE_C(); call backSlashN();
}
