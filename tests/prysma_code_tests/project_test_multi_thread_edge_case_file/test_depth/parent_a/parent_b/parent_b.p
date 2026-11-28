include "child_a.p";
include "child_b.p";
include "child_c.p";
include "child_d.p";
include "child_e.p";
include "parent_c/parent_c.p";

fn void printParentB()
{
    call printChildA_B(); call backSlashN();
    call printChildB_B(); call backSlashN();
    call printChildC_B(); call backSlashN();
    call printChildD_B(); call backSlashN();
    call printChildE_B(); call backSlashN();
    call printParentC(); call backSlashN();
}
