# Dt Engine
Игровой движок для студенческого проекта на языке C, построенный на базе архитектуры [ECS](https://github.com/SanderMertens/ecs-faq)

# Технологический стек
- Язык: C (стандарт C11)
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
``` C
DtEntity my_entity = dt_ecs_manager_new_entity(manager); //создаём новую сущность
DT_ECS_MANAGER_ADD_TO_POOL(manager, TagPoolType, my_entity, NULL); //добавляем её в набор тегов типа TagPoolType
DT_ECS_MANAGER_ADD_TO_POOL(manager, DataPoolType, my_entity, &(DataPoolType) {...});//добавляем её в набор компонентов типа DataPoolType
```

```C
DtEntity e1 = dt_ecs_manager_new_entity(manager);
DtEntity e2 = dt_ecs_manager_new_entity(manager); 
DtEntity e3 = dt_ecs_manager_new_entity(manager); 

DtEcsMask mask = dt_mask_new(manager, 0, 0); //создание маски
DT_MASK_EXC(mask, ExcComponent); //исключаем компонент ExcComponent
DtEcsFilter* filter = dt_mask_end(mask); //создаём фильтр - набор сущностей, у которых нет компонента ExcComponent

DT_ECS_MANAGER_ADD_TO_POOL(manager, NotExcComponent, e1, NULL);
DT_ECS_MANAGER_ADD_TO_POOL(manager, NotExcComponent, e2, NULL);
DT_ECS_MANAGER_ADD_TO_POOL(manager, ExcComponent, e3, NULL);

FOREACH(DtEntity, e, &filter->entities.entities_iterator, { 
  //проходимся по всем сущностям, у которых нет компонента ExcComponent - e1, e2
});
```

# Сборка и запуск
//TODO

# Статус разработки
- тесты для модулей
- редактор
- тесты для редактора
- демка небольшой игры
