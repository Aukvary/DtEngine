# Dt Engine
Игровой движок для студенческого проекта на языке C, построенный на базе архитектуры [ECS](https://github.com/SanderMertens/ecs-faq)

# Технологический стек
- Язык: C (стандарт C11)
- Компилятор: GCC/Clang
- Архитектура: Entity Component System (ECS)
- Графика: Raylib
- Сборка: CMake
- Зависимости:
  - [RayLib](https://github.com/raysan5/raylib) - для графики
  - [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) - для отладки 
  - [Nuklear-Raylib](https://github.com/RobLoach/raylib-nuklear) - биндинги для использования Nurlear c RayLib
  - [cJSON](https://github.com/DaveGamble/cJSON) - для парсинга json'ов

# Особенности
- ECS-архитектура создана на основе [leoecslite](https://github.com/Leopotam/ecslite)
- Разделение компонентов на данные и теги
  - тег - просто метка не хранит никаких данных(занимает меньше памяти)
  - компонент - полноценный компонент, который хранит данные об объекте
- Поддержка сборки под Windows и Linux
- использование json для хранения сериализованных данных
- редактор с использованием RayLib + Nuklear
- система модулей
  - разработчик может динамически подключать модули с наборами систем и компонентов и использовать их в своём проекте

# Пример использования

## Ecs Manager
- `DtEcsManager` - отвечает за создание/удаление сущностей, создание наборов и фильтров
- `DtEntity` - обычное беззнаковое 16-битное число, служит для получение информации о сущности
- `DtEntityInfo` - хранит информацию о сущности
```C
#include "Ecs/DtEcs.h"

DtEcsManagerConfig cfg = {
    .dense_size = 3,
    .sparse_size = 3,
    .recycle_size = 3,
    .components_count = 0,
    .pools_size = 0,
    .masks_size = 0,

    .children_size = 1,

    .include_mask_count = 0,
    .exclude_mask_count = 0,
    .filters_size = 0,
};

DtEcsManager* manager = dt_ecs_manager_new(cfg); // создание менеджера

DtEntity e1 = dt_ecs_manager_new_entity(manager); //создаём новую сущность - 0
DtEntity e2 = dt_ecs_manager_new_entity(manager); //создаём новую сущность - 1
DtEntity e3 = dt_ecs_manager_new_entity(manager); //создаём новую сущность - 2

DtEntityInfo info2 = dt_ecs_manager_get_entity(manager, e2) //получаем информацию о сущности

dt_ecs_manager_kill(manager, e2) //убиваем сущность, также она удаляется из всех DtEcsFilter и DtEcsPool
e2 = dt_ecs_manager_new_entity(manager); //имеет тот же индекс - 1, но другое поколоние,
                                         //то есть info2.gen != dt_ecs_manager_get_entity(manager, e2).gen
```
- Также `DtEcsManager` отвечает за управление иерархией
- при изменении иерархии обеем сущностям добавляет тег `DtHierachyDirty`
```C
dt_ecs_manager_set_parent(manager, e1, e2) // e1 становится дочерним объектом e2
dt_ecs_manager_add_child(manager, e1, e3) //добвляеет e3 к дочерним объектам e1
```

## Pools/Filters
- для реализации удобной системы модулей, создавать компоненты приходится через [X-макросы](https://www.geeksforgeeks.org/c/x-macros-in-c/)
```C
//объявляем макрос для инициализации и регистрации компонентов
//поле field1 типа int c атрибутом MyAttribute
//поле field2 типа char* без атрибутов
//поле field3 типа DtEntity c атрибутом типа int и char*
#define MY_COMPONENT(X, name)                       \ 
    X(int, field1, name,(DtAttributeData) {         \ 
      .attribute_name = "my_attr",                  \
      .data = &(MyAttribute) {...},                 \
    })                                              \
    X(char*, field2, name)                          \
    X(DtEntity, field3, name,                       \ 
      (DtAttributeData) {                           \ 
        .attribute_name = "int_attr",               \
        .data = &(int) {10},                        \
      },                                            \
      (DtAttributeData) {                           \ 
        .attribute_name = "str_attr",               \
        .data = &(char*) {"some info"},             \
      })                                            

DT_DEFINE_COMPONENT(MyComponent, MY_COMPONENT); //создаём тип структуры с именем MyComponent по шаблону MY_COMPONENT
```

- чтобы можно было получать метаданные компонента, необходимо зарегистрировать его в общем реестре, лучше это делать в любом `.c` файле, так как макрос для регестрации создаёт глобальные статические переменные
```C
DT_REGISTER_TAG(TagComponentWithAttrs, { //регестрируем тег с атрибутом my_tag_attr типа int
  .attribute_name = "my_tag_attr",
  .data = &(int) {10},
});

DT_REGISTER_TAG(TagComponentWithOutAttrs) //регестрируем тег без атрибутов

DT_REGISTER_COMPONENT(MyComponent, MY_COMPONENT); //регестрируем компонент без атрибутов
```

- `DtEcsPool` - набор сущностей, может содержать как теги, так и компоненты с данными
``` C
#include "Ecs/DtEcs.h"

DtEntity entity = dt_ecs_manager_new_entity(manager);
DT_ECS_MANAGER_ADD_TO_POOL(manager, TagPoolType, entity, NULL); //добавляем её в набор тегов TagPoolType
DT_ECS_MANAGER_ADD_TO_POOL(manager, DataPoolType, entity, &(DataPoolType) {/*данные*/}); //добавляем её в набор компонентов DataPoolType

DtEcsPool* tag_pool = DT_ECS_MANAGER_GET_POOL(manager, TagPoolType);
FOREACH(DtEntity, e, &tag_pool->iterator, {
  //проходимся по всем сущностям с компонентом TagPoolType
});

DtEcsPool* data_pool = DT_ECS_MANAGER_GET_POOL(manager, DataPoolType);
FOREACH(DtEntity, e, &data_pool->iterator, {
  //проходимся по всем сущностям с компонентом DataPoolType
});

DT_ECS_MANAGER_REMOVE_FROM_POOL(manager, DataPoolType, entity); //удаляем у entity компонент DataPoolType
```

- `DtEcsMask` - хранит данные о том, какие компоненты нужно включать, а какие исключать 
- `DtEcsFilter` - хранит все сущности, которые соответсвуют маске
```C
#include "Ecs/DtEcs.h"

DtEntity e1 = dt_ecs_manager_new_entity(manager);
DtEntity e2 = dt_ecs_manager_new_entity(manager); 
DtEntity e3 = dt_ecs_manager_new_entity(manager); 

DtEcsMask mask = dt_mask_new(manager, 0, 0); //создание маски
DT_MASK_INC(mask, IncComponent); //добавляем компонент IncComponent в маску
DT_MASK_EXC(mask, ExcComponent); //исключаем компонент ExcComponent в маску
DtEcsFilter* filter = dt_mask_end(mask); //создаём фильтр - набор сущностей, у которых есть компонент IncComponent нет компонента ExcComponent

//никак не меняем e1
DT_ECS_MANAGER_ADD_TO_POOL(manager, IncComponent, e2, NULL); //добавляем IncComponent
DT_ECS_MANAGER_ADD_TO_POOL(manager, ExcComponent, e3, NULL); //добавляем ExcComponent

FOREACH(DtEntity, e, &filter->entities.entities_iterator, { 
  //проходимся по всем сущностям, у которых есть компонент IncComponent нет компонента ExcComponent - e2
});

DT_ECS_MANAGER_ADD_TO_POOL(manager, IncComponent, e1, NULL); //добавляем IncComponent
DT_ECS_MANAGER_ADD_TO_POOL(manager, ExcComponent, e2, NULL); //добавляем ExcComponent
DT_ECS_MANAGER_ADD_TO_POOL(manager, IncComponent, e3, NULL); //добавляем IncComponent

FOREACH(DtEntity, e, &filter->entities.entities_iterator, { 
  //проходимся по всем сущностям, у которых есть компонент IncComponent нет компонента ExcComponent - e1
});
```

## Systems
- `DtUpdateHandler` - отвечает за обработку цикла обновления 
- `DtUpdateSystem` - обёртка над функциями обновления
```C
#include "Ecs/RegisterHandler.h"

typedef struct {         //созадие своей системы
  DtUpdateSystem system; //информация о системе
  DtEcsFilter* filter;   //пример данных
} MyUpdate;

DtUpdateSystem* my_update_new() {
    MyUpdate* update = DT_MALLOC(sizeof(MyUpdate)); //выделение памяти под систему

    *update = (MyUpdate) { //инициализация
        .system =
            (UpdateSystem) {
                .init = my_update_init,
                .update = my_update_update,
                .destroy = my_update_destroy,
                .priority = 0, //приоритет по сравнению с другими системами
                .data = update, //ссылка на данные системы
            },
    };

    return &update->system; //возвращаем общую информацию
}

void my_update_init(DtEcsManager* manager, void* data) {
  MyUpdate* update = data;
  //инициализруем фильтр...
}

void my_update_update(void* data, DtUpdateContext* ctx) {
    MyUpdate* update = data;

    FOREACH(DtEntity, e, &filter->entities.entities_iterator, { 
      //...
    });
}

void my_update_destroy(void* data) {
    MyUpdate* update = data;
    //очищаем данные
}

DT_REGISTER_UPDATE(MyUpdate, my_update_new); //регестрируем систему в общем реестре
                                             //лучше делать в .c файле, так как создаёт статические глобальные переменные
```

# Сборка и запуск
//TODO

# Над чем ведётся разработка 
- тесты для модулей
- редактор
- тесты для редактора
- демка небольшой игры
