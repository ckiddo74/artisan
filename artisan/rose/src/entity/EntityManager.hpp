#include <entity/Entity.hpp>
#include <map>
#include <string>
#include <boost/python.hpp>
#include <utils/hmsg.hpp>


class EntityManager {
public:    
   template <class T>
   static void register_entity () {
        using namespace entity;

        set_entity_info(T::_entity,
                        EntityInfo(T::check, 
                                   T::create,
                                   T::meta,
                                   (enum VariantT) T::_sg_variant));  
        set_sg_entity(T::_sg_type, T::_entity);    
   }

   static void register_entities();

   static std::list<std::string> get_entities(); 
   static EntityInfo *get_entity(std::string entity);
   static EntityInfo *expect_entity(std::string entity);   
   
   static std::string get_sg_entity(std::string sg_type);
   static std::string expect_sg_entity(std::string sg_type);
   static void set_sg_entity(std::string sg_type, std::string entity);

protected:
    static void set_entity_info(std::string entity, const EntityInfo &info);   
   
protected:
   // entity -> { check, create, rank, entities, sg_variant }
   static std::map<std::string, EntityInfo> _entity_info;
   // sg_type -> entity
   static std::map<std::string, std::string> _sg_entity;     
};
