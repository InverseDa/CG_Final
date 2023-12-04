#pragma once
#include <string>

class MgrBase {
  private:
    std::string name;

  public:
    MgrBase(std::string name = "MgrBase");
    virtual ~MgrBase();

    virtual std::string GetName();
};