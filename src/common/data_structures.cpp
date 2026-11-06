#include "data_structures.h"


void Attach(Link* link1, Link* link2) {
    assert(link1 || link2, "Two empty links tried to be linked.");
    if (link1) {
        link1->next = link2;
    }
    if (link2) {
        link2->previous = link1;
    }
}

void Delete(Link* this_link) {
    Attach(this_link->previous, this_link->next);
    this_link->previous = nullptr;
    this_link->next = nullptr;
}

Linked_List Concatenate(Linked_List list1, Linked_List list2) {
    Linked_List result = {};
    result.first = list1.first;
    Attach(list1.last, list2.first);
    result.last = list2.last;
    return result;
}