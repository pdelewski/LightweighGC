#pragma once

#include <cassert>
#include <set>
#include <map>

namespace ucore
{
  enum ptr_ownersip_property {OWNER, ALIAS};

  struct resource 
  {
    resource():counter(0) {}
    int counter;
    #ifdef DEBUG
    std::set<std::pair<std::string,size_t>> aliases_locations;
    std::pair<std::string,size_t> owner_location;
    #endif
  };

  template<typename T>
  struct gen_ptr
  {
    gen_ptr(const std::string& file = std::string(),
              const size_t line = 0):owner(true), ptr(nullptr) {}
    gen_ptr(T* p,const std::string& file = std::string(),
              const size_t line = 0):owner(false), ptr(p) {}
    explicit gen_ptr(bool owner, T *p, const std::string& file = std::string(),
                      const size_t line = 0) :owner(owner), ptr(p)
    {
      if (ptr && !owner)
      {
        ++ptr->counter;
      }
    }
    
    gen_ptr(const gen_ptr& rhs)
    {
      assert(!rhs.owner);
      ptr = rhs.ptr;
      if (ptr && !owner)
      {
        ++ptr->counter;
      }
    }

    gen_ptr& operator=(const gen_ptr& rhs)
    {
      if (&rhs == this) {
        return *this;
      }

      assert ((owner && rhs.is_owner())==false);

      if (ptr && !owner) {
        --ptr->counter;
      }
      ptr = rhs.ptr;
      if (ptr && !owner) {
        ++ptr->counter;
      }
      return *this;
    }
    
    ~gen_ptr()
    {
      if (!ptr) 
      {
        return;
      }
      if (ptr && !owner) {
        --ptr->counter;
      }
      if (owner) {
        assert(ptr->counter == 0);
        delete ptr;
      }
    }

    T* operator->() {
      return ptr;
    }

    operator bool() {
      return ptr != 0;
    }
      
    bool is_owner() const
    {
      return owner;
    }

    void set_ownership()
    {
      owner = true;
      if (ptr) {
        --ptr->counter;
      }
    }
    void convert_to_alias()
    {
      owner = false;
      if (ptr) {
        ++ptr->counter;
      }
    }

    void move_ownership_from(gen_ptr<T>& rhs)
    {
      rhs.convert_to_alias();
      *this = rhs;
    }
    int alias_counter() const {
      return ptr->counter;
    }
  private:
    bool owner;
    T* ptr;
  };

  template<typename T>
  auto make_owning_ptr(T *ptr,
                      const std::string& file = std::string(),
                      const size_t line = 0) -> gen_ptr<T>
  {
    return gen_ptr<T>(true, ptr, file, line);
  }

  template<typename T>
  auto make_alias(T *ptr = nullptr,
                  const std::string& file = std::string(),
                  const size_t line = 0) -> gen_ptr<T>
  {
    return gen_ptr<T>(false, ptr, file, line);
  }
}
