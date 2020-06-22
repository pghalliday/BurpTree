#include <unity.h>
#include <functional>
#include "../src/BurpRedux.hpp"
#include "Subscriber.hpp"
#include "Selector.hpp"

namespace BurpReduxTest {

  using namespace BurpRedux;

  class Child : public BurpRedux::State<Child> {
    public:
      bool isNew(const Child * current) const override {
        return (this != current);
      }
  };

  class Parent : public BurpRedux::State<Parent> {
    public:
      const Child * child;
      Parent(const Child * child) :
        child(child)
      {}
      bool isNew(const Parent * current) const override {
        return (this != current);
      }
  };

  using Publisher = BurpRedux::Publisher::Instance<Parent, 1>;
  using Selector = BurpRedux::Selector::Instance<Parent, Child, 1>;

  Child c1;
  Child c2;
  Parent p1(&c1);
  Parent p2(&c1);
  Parent p3(&c2);

  Publisher publisher;
  Selector selector([](const Parent * parent) {
      return parent->child;
  });
  Subscriber<Child> childSubscriber;

  const Child * callbackChild;

  Module selectorTests("Selector", [](Describe & describe) {
      describe.setup([]() {
          publisher.subscribe(&selector);
          selector.subscribe(&childSubscriber);
          publisher.setup(&p1);
          selector.setup(publisher.getState());
      });

      describe.describe("with the initial state", [](Describe & describe) {
          describe.it("should have the correct state", []() {
              TEST_ASSERT_EQUAL(&c1, selector.getState());
          });

          describe.describe("then with a new parent but the same child", [](Describe & describe) {
              describe.before([]() {
                  callbackChild = nullptr;
                  childSubscriber.callbackOnce([&](const Child * child) {
                      callbackChild = child;
                  });
                  publisher.publish(&p2);
              });

              describe.it("should not notify or change state", []() {
                  TEST_ASSERT_NULL(callbackChild);
                  TEST_ASSERT_EQUAL(&c1, selector.getState());
              });

              describe.describe("then with a new parent and a new child", [](Describe & describe) {
                  describe.before([]() {
                      callbackChild = nullptr;
                      childSubscriber.callbackOnce([&](const Child * child) {
                          callbackChild = child;
                      });
                      publisher.publish(&p3);
                  });

                  describe.it("should notify and change state", []() {
                      TEST_ASSERT_EQUAL(&c2, callbackChild);
                      TEST_ASSERT_EQUAL(&c2, selector.getState());
                  });
              });
          });
      });
  });

}
