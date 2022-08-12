#include <iostream>
#include <random>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.

    std::lock_guard<std::mutex> lck2(_mutex2);

    std::unique_lock<std::mutex> ulock(_mutex);
    _cond.wait(ulock, [this]()
               { return !_queue.empty(); });
    
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> loc(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    // _time = std::chrono::high_resolution_clock::now();
}

TrafficLight::~TrafficLight()
{
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        // TrafficLightPhase received = _message.receive();
        if (_message.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.

    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
    // std::cout<<"Hello from Simulate \n";
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    /* Generate random number between 4 and 6 seconds */
    unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> dist(4000, 6000);
    double random_duration = dist(gen);

    // int range = 6 - 4 + 1;
    // double num = rand() % range + 4;
    // std::cout << "Num is " << num << std::endl;

    auto time_prev = std::chrono::high_resolution_clock::now();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::lock_guard lck2(_mutex2);
        auto time_now = std::chrono::high_resolution_clock::now();
        
        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_prev).count();
        // random_duration = 4.0;
        if (time_diff >= random_duration)
        {
            if (_currentPhase == red)
            {
                _currentPhase = green;
            }
            else if (_currentPhase == green)
            {
                _currentPhase = red;
            }
            time_prev = std::chrono::high_resolution_clock::now();
            _message.send(std::move(_currentPhase));
        }
    }
}
