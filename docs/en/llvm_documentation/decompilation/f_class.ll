source_filename = "Class.cpp"
target triple = "x86_64-pc-linux-gnu"

; The multiple inheritance system works visually like an onion technique
; classes are stacked on top of each other, each child class contains an instance of its parent class,
; thus, we can access the methods and attributes of the parent class through the instance

; The steps to instantiate a constructor of a class with multiple inheritance are as follows:
; The Cat constructor calls IAnimal_constructor.
; IAnimal_constructor calls LivingBeing_constructor.
; LivingBeing_constructor executes, then returns control to IAnimal.
; IAnimal sets up its VTable, then returns control to Cat.
; Cat finally sets up its final VTable.

%class.IAnimal = type { ptr }

%class.Cat = type <{ 
  %class.IAnimal,
  i32
}>

@VTable_IAnimal = constant [1 x ptr] [
  ptr @IAnimal_destructor,
  ptr @__shout_virtual   ; a virtual method for the IAnimal class which 
  ; will be overridden by the Cat class 
]

; The vtable is a table of function pointers that enables polymorphism in C++, it contains the addresses
; of the class's virtual methods, so we can call the class's methods through the vtable,
; like the destructor, shout, cat, everything that is in the cat class
; we make a vtable for each class, so the cat class has its own vtable containing the methods of the cat class, as well as the methods of the parent class IAnimal.

@VTable_Cat = constant [2 x ptr] [ 
  ptr @Cat_destructor,
  ptr @Cat_shout
]

define i32 @main() {
entry:
  %myCat = alloca %class.Cat, align 8 ; reserve memory for an object of type Cat

  call void @Cat_constructor(ptr %myCat, i32 3) ; call the constructor of the Cat class to initialize the myCat object with an age of 3

  %vtable_ptr_ptr = getelementptr inbounds %class.Cat, ptr %myCat, i32 0, i32 0, i32 0 

  %vtable_ptr = load ptr, ptr %vtable_ptr_ptr, align 8 ; load the address of the cat class's vtable into the variable vtable_ptr, so we can access the cat class's methods through the vtable

  %shout_func_ptr_ptr = getelementptr inbounds ptr, ptr %vtable_ptr, i64 1 
  %shout_func_ptr = load ptr, ptr %shout_func_ptr_ptr, align 8 ; load the address of the shout method into the variable shout_func_ptr 

  call void %shout_func_ptr(ptr %myCat) ; we can now call the shout method of the cat class through the vtable stored in the memory allocation
  
  call void @Cat_destructor(ptr %myCat) 

  ret i32 0
}

define void @Cat_constructor(ptr %this, i32 %age) { ; like the destructor, we use the onion method to manage instantiation order
entry:
  call void @IAnimal_constructor(ptr %this)

  %vtable_ptr_ptr = getelementptr inbounds %class.Cat, ptr %this, i32 0, i32 0, i32 0
  store ptr getelementptr ([2 x ptr], ptr @VTable_Cat, i32 0, i32 0), ptr %vtable_ptr_ptr, align 8

  %age_ptr = getelementptr inbounds %class.Cat, ptr %this, i32 0, i32 1
  store i32 %age, ptr %age_ptr, align 8
  ret void
}

define void @Cat_shout(ptr %this) { 
entry:
  ret void
}

define void @Cat_destructor(ptr %this) { ; destructor method of the cat class
entry:
  call void @IAnimal_destructor(ptr %this) ; we use a destructor method because 
  ; it's an onion technique, we destroy everything from child to parent,
  ; This is where we manage the destruction order, we destroy children first then parents
  ; otherwise we risk memory leaks or undefined behavior if we destroy parents first
  ret void
}

define void @IAnimal_constructor(ptr %this) {
entry:
  %vtable_ptr_ptr = getelementptr inbounds %class.IAnimal, ptr %this, i32 0, i32 0
  store ptr getelementptr ([1 x ptr], ptr @VTable_IAnimal, i32 0, i32 0), ptr %vtable_ptr_ptr, align 8
  ret void
}

; Each destructor has an associated method to manage the destruction order
define void @IAnimal_destructor(ptr %this) {
entry:
  ret void
}

