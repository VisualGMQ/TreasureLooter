@0xaa6fbf2bba42a34f;

using Person = import "person.capnp";

struct Test {
    person @0: Person.Person;
}