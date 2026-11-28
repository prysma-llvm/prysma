include "child_a.p";
include "child_b.p";
include "child_c.p";
include "child_d.p";
include "child_e.p";
include "parent_b/parent_b.p";

fn void printParentA()
{
    call printChildA(); call backSlashN();
    call printChildB(); call backSlashN();
    call printChildC(); call backSlashN();
    call printChildD(); call backSlashN();
    call printChildE(); call backSlashN();
    call printParentB(); call backSlashN();
}