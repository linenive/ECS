/*
Copyright (c) 2016 Sam Bloomberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

본 소프트웨어 및 관련 문서 파일(이하 “소프트웨어”)의 사본을 취득하는 모든 사람은
소프트웨어를 제한 없이 취급할 수 있는 권한을 무료로 부여받습니다. 여기에는 소프트웨어의 사용, 복사, 수정, 병합, 출판, 배포, 재라이선스 및/또는 판매에 대한
권리를 포함하되 이에 국한되지 않는 제한 없는 소프트웨어의 취급이 포함됩니다.
또한 소프트웨어가 제공되는 사람이 이를 수행할 수 있도록 허용하는 조건은 다음과 같습니다.
다음 조건을 따릅니다.

상기 저작권 고지 및 본 허가 고지는 소프트웨어의 모든
복사본 또는 실질적인 부분에 포함되어야 합니다.

소프트웨어는 “있는 그대로” 제공되며, 상품성, 특정 목적에의 적합성 및 비침해성에 대한 보증을 포함하되 이에 국한되지 않는 명시적 또는 묵시적 보증 없이 제공됩니다.
상업성, 특정 목적에의 적합성 및 비침해성에 대한 보증을 포함하되 이에 국한되지 않는 명시적 또는 묵시적 보증 없이 제공됩니다. 어떠한 경우에도 저자나 저작권 보유자는
계약, 불법행위 또는 기타의 소송에서 발생하는 모든 청구, 손해 또는 기타
책임에 대해 책임을 지지 않습니다. 소프트웨어 또는 소프트웨어의 사용 또는 기타 거래와 관련하여 발생하는
소프트웨어의 사용 또는 기타 거래와 관련하여 발생하는 모든 청구, 손해 또는 기타 책임에 대해 책임을 지지 않습니다.
*/

#include <iostream>

#include "ECS.h"

ECS_TYPE_IMPLEMENTATION;

using namespace ECS;

struct Position
{
	ECS_DECLARE_TYPE;

	Position(float x, float y) : x(x), y(y) {}
	Position() {}

	float x;
	float y;
};

ECS_DEFINE_TYPE(Position);

struct Rotation
{
	ECS_DECLARE_TYPE;

	Rotation(float angle) : angle(angle) {}
	Rotation() {}

	float angle;
};

ECS_DEFINE_TYPE(Rotation);

struct SomeComponent
{
	ECS_DECLARE_TYPE;

	SomeComponent() {}
};

ECS_DEFINE_TYPE(SomeComponent);

struct SomeEvent
{
	ECS_DECLARE_TYPE;

	int num;
};

ECS_DEFINE_TYPE(SomeEvent);

class TestSystem : public EntitySystem,
	public EventSubscriber<Events::OnEntityCreated>,
	public EventSubscriber<Events::OnEntityDestroyed>,
	public EventSubscriber<Events::OnComponentRemoved<Position>>,
	public EventSubscriber<Events::OnComponentRemoved<Rotation>>,
	public EventSubscriber<SomeEvent>
{
public:
	virtual ~TestSystem() {}

	virtual void configure(class World* world) override
	{
		world->subscribe<Events::OnEntityCreated>(this);
		world->subscribe<Events::OnEntityDestroyed>(this);
		world->subscribe<Events::OnComponentRemoved<Position>>(this);
		world->subscribe<Events::OnComponentRemoved<Rotation>>(this);
		world->subscribe<SomeEvent>(this);
	}

	virtual void unconfigure(class World* world) override
	{
		world->unsubscribeAll(this);
	}

	virtual void tick(class World* world, float deltaTime) override
	{
		world->each<Position, Rotation>([&](Entity* ent, ComponentHandle<Position> pos, ComponentHandle<Rotation> rot) -> void {
			pos->x += deltaTime;
			pos->y += deltaTime;
			rot->angle += deltaTime * 2;
		});
	}

	virtual void receive(class World* world, const Events::OnEntityCreated& event) override
	{
		std::cout << "An entity was created!" << std::endl;
	}

	virtual void receive(class World* world, const Events::OnEntityDestroyed& event) override
	{
		std::cout << "An entity was destroyed!" << std::endl;
	}

	virtual void receive(class World* world, const Events::OnComponentRemoved<Position>& event) override
	{
		std::cout << "A position component was removed!" << std::endl;
	}

	virtual void receive(class World* world, const Events::OnComponentRemoved<Rotation>& event) override
	{
		std::cout << "A rotation component was removed!" << std::endl;
	}

	virtual void receive(class World* world, const SomeEvent& event) override
	{
		std::cout << "I received SomeEvent with value " << event.num << "!" << std::endl;

		// Let's delete an entity while iterating because why not?
		world->all([&](Entity* ent) {
			if (ent->getEntityId() + 1 == event.num)
				world->destroy(world->getById(event.num));

			if (ent->getEntityId() == event.num)
				std::cout << "Woah, we shouldn't get here!" << std::endl;
		});
	}
};

int main(int argc, char** argv)
{
	std::cout << "EntityComponentSystem Test" << std::endl
		<< "==========================" << std::endl;

	World* world = World::createWorld();

	EntitySystem* testSystem = world->registerSystem(new TestSystem());

	Entity* ent = world->create();
	auto pos = ent->assign<Position>(0.f, 0.f);
	auto rot = ent->assign<Rotation>(0.f);

	std::cout << "Initial values: position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")" << std::endl;

	world->tick(10.f);

	std::cout << "After tick(10): position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")" << std::endl;

	world->disableSystem(testSystem);

	world->tick(10.f);

	std::cout << "After tick(10) and DisableSystem(testSystem): position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")" << std::endl;

	world->enableSystem(testSystem);

	world->tick(10.f);

	std::cout << "After tick(10) and EnableSystem(testSystem): position(" << pos->x << ", " << pos->y << "), rotation(" << rot->angle << ")" << std::endl;

	ent->remove<Position>();
	ent->remove<Rotation>();

	std::cout << "Creating more entities..." << std::endl;

	for (int i = 0; i < 10; ++i)
	{
		ent = world->create();
		ent->assign<SomeComponent>();
	}

	int count = 0;
	std::cout << "Counting entities with SomeComponent..." << std::endl;
	// range based for loop
	for (auto ent : world->each<SomeComponent>())
	{
		++count;
		std::cout << "Found entity #" << ent->getEntityId() << std::endl;
	}
	std::cout << count << " entities have SomeComponent!" << std::endl;

	// Emitting events
	world->emit<SomeEvent>({ 4 });

	std::cout << "We have " << world->getCount() << " entities right now." << std::endl;
	world->cleanup();
	std::cout << "After a cleanup, we have " << world->getCount() << " entities." << std::endl;

	std::cout << "Destroying the world..." << std::endl;

	world->destroyWorld();

	std::cout << "Press any key to exit..." << std::endl;
	std::getchar();

	return 0;
}