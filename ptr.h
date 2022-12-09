#pragma once

#include <cassert>
#include <map>
#include <utility>
#include <iostream>

namespace ucore
{
  enum ptr_ownersip_property {OWNER, ALIAS};

  struct resource 
  {
    resource():counter(0) {}
    int counter;
    #ifdef DEBUG
      std::map<size_t, std::pair<std::string,size_t>> aliases_locations;
      std::pair<std::string,size_t> owner_location;
    #endif
  };

  template<typename T>
  struct gen_ptr
  {
    gen_ptr(const std::string& file = std::string("undefined"),
              const size_t line = 0):owner(true), ptr(nullptr)
    {
      init_source_location(file, line);
    }
    gen_ptr(T* p,const std::string& file = std::string("undefined"),
              const size_t line = 0):owner(false), ptr(p)
    {
      init_source_location(file, line);
    }
    explicit gen_ptr(bool owner, T *p, const std::string& file = std::string("undefined"),
                      const size_t line = 0) :owner(owner), ptr(p)
    {
      if (ptr && !owner)
      {
        ++ptr->counter;
      }
      init_source_location(file, line);
    }
    
    gen_ptr(const gen_ptr&& rhs)
      :owner(rhs.owner) ,ptr(rhs.ptr), file(rhs.file), line(rhs.line)
    {
      init_source_location(file, line);
    }

    gen_ptr& operator=(gen_ptr&& rhs)
    {
      remove_source_location();
      if (ptr && !owner) {
        --ptr->counter;
      }
      owner = std::move(rhs.owner);
      ptr = std::move(rhs.ptr);

      move_source_location();
      return *this;
    }

    gen_ptr(const gen_ptr& rhs)
    {
      assert(!rhs.owner);
      ptr = rhs.ptr;
      owner = rhs.owner;
      file = rhs.file;
      line = rhs.line;
      init_source_location(file, line);
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
      remove_source_location();
      assert ((owner && rhs.is_owner())==false);
      if (ptr && !owner) {
        --ptr->counter;
      }

      ptr = rhs.ptr;

      if (ptr && !owner) {
        ++ptr->counter;
      }
      move_source_location();
      return *this;
    }
    
    ~gen_ptr()
    {
      remove_source_location();
      if (ptr && !owner) {
        --ptr->counter;
      }

      if (ptr && owner) {
        dump_live_aliases();
        assert(ptr->counter == 0);
        delete ptr;
      }
    }

    T* operator->() {
      return ptr;
    }

    bool operator<(const gen_ptr& rhs) const{
      return ptr < rhs.ptr;
    }

    operator bool() {
      return ptr != 0;
    }

    gen_ptr& with_source_location(const std::string& f, size_t l)
    {
      #ifdef DEBUG
        file = f;
        line = l;
      #endif
      return *this;
    }

    bool is_owner() const
    {
      return owner;
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
    void init_source_location(const std::string& file, size_t line)
    {
      #ifdef DEBUG
        this->file = file;
        this->line = line;
        if (ptr) {
          if (owner) {
            ptr->owner_location = std::make_pair(file,line);
          } else {
            ptr->aliases_locations.insert(std::make_pair((size_t)this, std::make_pair(file,line)));
          }
        }
      #endif
    }
    void remove_source_location()
    {
      #ifdef DEBUG
        if (ptr)
        {
          auto it = ptr->aliases_locations.find(size_t(this));
          if (it != ptr->aliases_locations.end()) {
            ptr->aliases_locations.erase(it);
          }
        }
      #endif
    }
    void move_source_location()
    {
      #ifdef DEBUG
        if (ptr) {
          if (owner) {
            ptr->owner_location = std::make_pair(file,line);
          } else {
            ptr->aliases_locations.insert(std::make_pair((size_t)this, std::make_pair(file,line)));
          }
        }
      #endif
    }
    void dump_live_aliases()
    {
      #ifdef DEBUG
        if (ptr->counter != 0) {
          std::cout << "owner : " << ptr->owner_location.first << ":"
                                  << ptr->owner_location.second << std::endl;
          for (const auto& alias : ptr->aliases_locations) {
            std::cout << "alias : " << alias.second.first << ":"
                                    << alias.second.second << std::endl;
          }
        }
      #endif
    }
    bool owner;
    T* ptr;
    #ifdef DEBUG
      std::string file;
      size_t line;
    #endif
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
