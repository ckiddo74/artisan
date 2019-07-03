#include <entity/EntityManager.hpp>
using namespace std;

map<string, EntityInfo> EntityManager::_entity_info;
map<string, string> EntityManager::_sg_entity;  

#define REG_ENTITY(class) EntityManager::register_entity<entity::class>();

#include <entity_defs.hpp>
void EntityManager::register_entities() {
    REGISTER_ENTITIES   
  
    // compute meta-information
    for (auto &entity : _entity_info) {
        entity.second.compute_meta();
    }

#if 0
    for (auto x : _entity_info) {
        printf("====> %s [%d]\n", x.first.c_str(), x.second.meta_rank);
        std::string s;
        for (const auto &piece : x.second.meta_entities) s += piece +",";
        printf("   - %s\n", s.c_str());
        for (auto y: x.second.meta_doc) {
           EntityDoc &doc = y.second;  
           if (doc.is_method) {
              printf("   %s(%s): %s\n", y.first.c_str(), doc.args.c_str(), doc.desc.c_str());
           } else {
              printf("   %s: %s\n", y.first.c_str(), doc.desc.c_str());
           }
        }
    }
#endif    
}


list<string> EntityManager::get_entities() {
    list<string> entities;
    for (auto e: _entity_info) {
        entities.push_back(e.first);
    }
    return entities;
}

EntityInfo *EntityManager::get_entity(string entity) {
    EntityInfo *info = 0;    
    map<string, EntityInfo>::iterator it = _entity_info.find(entity);
    if (it != _entity_info.end()) {
        info = &it->second;
    }         

    return info;
}

EntityInfo *EntityManager::expect_entity(string entity) {
    EntityInfo *info = get_entity(entity);
    hAssert(info, "internal error: expecting entity '%s' in EntityManager!", entity);
    return info;
}

string EntityManager::get_sg_entity(string sg_type) {
    string entity;
    map<string, string>::iterator it = _sg_entity.find(sg_type);
    if (it == _sg_entity.end()) {
        entity = "";
    } else {
        entity = it->second;
    }     
    return entity;
}

string EntityManager::expect_sg_entity(string sg_type) {
    string entity = get_sg_entity(sg_type);
    hAssert(!entity.empty(), "internal error: cannot find entity for sgnode: %s!", sg_type);
    return entity;
}

void EntityManager::set_sg_entity(string sg_type, string entity) {
    hAssert(_sg_entity.count(sg_type) == 0, "internal error: SgNode [%s] has already been associated to entity [%s]!", sg_type, entity);
    _sg_entity[sg_type] = entity;
}

void EntityManager::set_entity_info(string entity, const EntityInfo &info) {
   hAssert(_entity_info.count(entity) == 0, "internal error: entity [%s] can only be registered once!", entity);
   _entity_info.insert(pair<string, EntityInfo>(entity, info));
}

