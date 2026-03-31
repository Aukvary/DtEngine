# Dt Engine
Игровой движок для студенческого проекта на языке C, построенный на базе архитектуры [ECS](https://github.com/SanderMertens/ecs-faq)

# Технологический стек
- Язык: C (стандарт C11)
- Компилятор: GCC/Clang
- Архитектура: Entity Component System (ECS)
- Графика: Raylib
- Сборка: CMake/Ninja
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

# Сборка и запуск
- сначала необходимо получить исходники проекта и запустить скрипт для сборки
```
git clone https://github.com/Aukvary/DtEngine.git
cd DtEngine
```
- для Linux
```
./debug_build.sh 
```
- для Windows
```
.\debug_build.bat 
```

## Особенности
- вы можете передать название таргета, который вы хотите собрать
- проект собирается в несколько директорий в директории build:
  - core -  статическая библиотека игры и тесты для неё
  - editor - редактор, редактор api и динамическая библиотека игры
  - game - готовая игра и статическая библиотека игры

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

## Modules
- `DtModuleInfo` - хранит информацию о модуле(компоненты, системы, сцены)
- `DtEnvironment` - хранит информацию о компонентах, системах и сценах
``` C
ModuleInfo* lib = dt_module_load(dt_environment_instance(), DT_LIB_NAME("./my_lib")); //загрузка библиотеки

components = game_lib->environment->components; //получение всех компонентов
components_count = game_lib->environment->components_count; //количество компонентов

updates = game_lib->environment->updaters; //получение всех систем обновления
updates_count = game_lib->environment->updaters_count; //количество систем обновления

draws = game_lib->environment->drawers; //получение всех систем отрисовки
draws_count = game_lib->environment->drawers_count; //количество систем отрисовки

void* custom_data = DT_LIB_GET(lib->hadndler, "my_data_name"); //получение данных

dt_module_unload(dt_environment_instance(), lib); //выгрузка библиотеки
```

# Редактор(WIP)
## взаимодействие с редаетором 
- `DtEFuncTable` - таблица функций для взаимодействие с редаетором
  - `environment_instance` - получение окружения редактора
  
  - `log` - вывести сообщение в окно сообщений редактора
  - `warn` - вывести предупреждение в окно сообщений редактора
  - `error` - вывести ошибку в окно сообщений редактора

  - `add_parser_json_to_type` - добавить парсер json'a в тип
  - `add_serializer_type_to_json` - добавить парсер типа в  json
  - `add_inspector_type` - добавить поле отрисовки для типа
  - `link_parser_json_to_type` - слинковать функциии для парсинга типов 
```C
DECLARE_EDITOR_FUNC_TABLE //определяем глобальную переменную, которая хранит нужные функции

func_table.log("entity(ID: %d) was created", my_entity);
func_table.warn("entity(ID: %d) was killed again", my_entity);
func_table.error("entity(ID: %d) out of range" my_entity);

void parse_json_to_my_type(cJSON* json, void* data);
func_table.add_parser_json_to_type("MyType", parse_json_to_my_type);

cJSON* serialize_my_type_to_json(const void* data);
func_table.add_serializer_type_to_json(serialize_my_type_to_json);

bool inspector_field_my_type(const char* field_name, void* data); //возвращает было изменено значение или нет
func_table.add_inspector_type(inspector_field_my_type);

void link_my_type_n_another_type(const char* type, const char* base_type);
func_table.link_parser_json_to_type(link_my_type_n_another_type);
```

## атрибуты
### компоненты
- `DTE_INSPECTOR_HIDE` - скрыть поле в редакторе
- `DTE_ON_FIELD_CHANGE` - событие на изменение поля в редакторе

```C
void on_field_change(
  DtEcsPool* data, //набор компонентов, у которого изменилось поле 
  DtEntity entity // сущность, у которой изменилось поле
);

#define MY_COMPONENT(X, name)                                                               \
    X(Vector2, observer_field, name, DTE_ON_FIELD_CHANGE(on_field_change))                  \
    X(int, hidden_field, name, DTE_INSPECTOR_HIDE) 
```
### системы
- `DTE_EDIT_UPDATE` - обновление в режиме редактирования
- `DTE_EDIT_DRAW` - отрисовка в режиме редактирования

```C
DtUpdateSystem* my_update_system_new();
DtUpdateSystem* my_editor_update_system_new();
DT_REGISTER_UPDATE(MyUpdateSystem, my_update_system_new, DTE_EDIT_UPDATE(my_editor_update_system_new))

DtDrawSystem* my_draw_system_new();
DtDrawSystem* my_editor_draw_system_new();
DT_REGISTER_DRAW(MyDrawSystem, my_draw_system_new, DTE_EDIT_DRAW(my_editor_draw_system_new))
```

# Над чем ведётся разработка
- редактор
- тесты для редактора
- демка небольшой игры
