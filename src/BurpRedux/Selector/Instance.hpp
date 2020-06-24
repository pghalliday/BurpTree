#pragma once

#include <functional>
#include "../Publisher/Instance.hpp"
#include "Interface.hpp"

namespace BurpRedux {
  namespace Selector {

    template <class Input, class Output, size_t size>
    class Instance : public Interface<Input, Output> {
      
      public:

        using Subscriber = Subscriber::Interface<Output>;
        using Subscribers = std::array<Subscriber *, size>;
        using f_select = std::function<const Output * (const Input * input)>;

        Instance(f_select select, Subscribers subscribers) :
          select(select),
          outputPublisher(subscribers)
        {}

        void setup(const Input * input) override {
          outputPublisher.setup(select(input));
        }

        void onPublish(const Input * input) override {
          publish(select(input));
        }

        const Output * getState() const override {
          return outputPublisher.getState();
        }
        
      private:

        f_select select;
        Publisher::Instance<Output, size> outputPublisher;

        void publish(const Output * output) override {
          outputPublisher.publish(output);
        }

    };


  }
}
