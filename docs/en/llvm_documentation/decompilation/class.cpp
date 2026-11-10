//===-- class_example.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

class IAnimal 
{
public:
    virtual ~IAnimal() = default;
    virtual void shout() = 0;
};

class Cat : public IAnimal
{
private: 
    int age_;
public: 
    Cat(int age) : age_(age) {}

    void shout() override {
       age_++;
    }
};

int main() {
    Cat myCat(3);
    myCat.shout();
    return 0;
}
