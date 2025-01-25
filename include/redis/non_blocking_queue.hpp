#include <atomic>
#include <iostream>
#include <memory>

template <typename T>
class NonBlockingQueue {
private:
    struct Node {
        T data;                             // The data stored in the node
        std::atomic<Node*> next{nullptr};   // Pointer to the next node (atomic for thread safety)

        explicit Node(T value) : data(std::move(value)), next(nullptr) {}
    };

    std::atomic<Node*> head;  // Pointer to the front of the queue
    std::atomic<Node*> tail;  // Pointer to the back of the queue

    Node* stub;  // A dummy node used for initialization

public:
    // Constructor
    NonBlockingQueue() {
        stub = new Node(T{});  // Create a dummy node
        head.store(stub, std::memory_order_relaxed);
        tail.store(stub, std::memory_order_relaxed);
    }

    // Destructor
    ~NonBlockingQueue() {
        // Clean up all nodes in the queue
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }

    // Enqueue: Add an item to the queue
    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        Node* currentTail = nullptr;

        while (true) {
            currentTail = tail.load(std::memory_order_acquire);
            Node* next = currentTail->next.load(std::memory_order_acquire);

            if (next == nullptr) {
                // Try to link the new node at the tail
                if (currentTail->next.compare_exchange_weak(next, newNode, std::memory_order_release, std::memory_order_relaxed)) {
                    break;  // Successfully linked
                }
            } else {
                // Tail pointer is lagging, try to advance it
                tail.compare_exchange_weak(currentTail, next, std::memory_order_release, std::memory_order_relaxed);
            }
        }

        // Try to advance the tail pointer to the new node
        tail.compare_exchange_weak(currentTail, newNode, std::memory_order_release, std::memory_order_relaxed);
    }

    // Dequeue: Remove and return an item from the queue
    bool dequeue(T* result) {
        Node* currentHead = nullptr;

        while (true) {
            currentHead = head.load(std::memory_order_acquire);
            Node* next = currentHead->next.load(std::memory_order_acquire);

            if (next == nullptr) {
                // Queue is empty
                return false;
            }

            // Try to move the head pointer forward
            if (head.compare_exchange_weak(currentHead, next, std::memory_order_release, std::memory_order_relaxed)) {
                *result = std::move(next->data);  // Extract the value into the provided pointer
                delete currentHead;              // Free the old head
                return true;
            }
        }
    }
};
