#include "mgr/mgr_base.hpp"

MgrBase::MgrBase(std::string name) : name(name) {
}

MgrBase::~MgrBase() {
}

std::string MgrBase::GetName() {
    return this->name;
}
