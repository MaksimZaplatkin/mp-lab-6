#include <gtest/gtest.h>
#include <vector>
#include <stdexcept>
#include <limits>

template <typename T, int D = 4>
class DHeap {
private:
    struct Node {
        T key;
        int vertex;
    };

    std::vector<Node> heap;
    std::vector<int> pos;

    int parent(int i) const { return (i - 1) / D; }
    int child(int i, int k) const { return D * i + k + 1; }

    void swapNodes(int i, int j) {
        Node temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
        pos[heap[i].vertex] = i;
        pos[heap[j].vertex] = j;
    }

    void siftUp(int i) {
        while (i > 0 && heap[i].key < heap[parent(i)].key) {
            swapNodes(i, parent(i));
            i = parent(i);
        }
    }

    void siftDown(int i) {
        while (true) {
            int smallest = i;
            int firstChild = child(i, 0);

            if (firstChild >= (int)heap.size()) break;

            int lastChild = firstChild + D;
            if (lastChild > (int)heap.size()) lastChild = heap.size();

            for (int c = firstChild; c < lastChild; ++c) {
                if (heap[c].key < heap[smallest].key) {
                    smallest = c;
                }
            }

            if (smallest == i) break;
            swapNodes(i, smallest);
            i = smallest;
        }
    }

public:
    DHeap(int maxVertices) : pos(maxVertices, -1) {}

    void push(int vertex, T key) {
        if (vertex >= (int)pos.size()) {
            pos.resize(vertex + 1, -1);
        }

        if (contains(vertex)) {
            decreaseKey(vertex, key);
            return;
        }

        pos[vertex] = heap.size();
        Node newNode = { key, vertex };
        heap.push_back(newNode);
        siftUp(heap.size() - 1);
    }

    bool empty() const { return heap.empty(); }

    bool contains(int vertex) const {
        return vertex < (int)pos.size() && pos[vertex] != -1;
    }

    int popMin() {
        if (heap.empty()) {
            throw std::runtime_error("Heap is empty");
        }

        int vertex = heap[0].vertex;
        pos[vertex] = -1;

        heap[0] = heap.back();
        heap.pop_back();

        if (!heap.empty()) {
            pos[heap[0].vertex] = 0;
            siftDown(0);
        }

        return vertex;
    }

    void decreaseKey(int vertex, T newKey) {
        if (!contains(vertex)) {
            push(vertex, newKey);
            return;
        }

        int i = pos[vertex];
        if (newKey < heap[i].key) {
            heap[i].key = newKey;
            siftUp(i);
        }
    }

    T getKey(int vertex) const {
        if (!contains(vertex)) {
            throw std::runtime_error("Vertex not in heap");
        }
        return heap[pos[vertex]].key;
    }

    size_t size() const { return heap.size(); }
};

template <typename T>
std::vector<T> dijkstra(int start, const std::vector<std::vector<std::pair<int, T>>>& graph) {
    int n = graph.size();
    T INF = std::numeric_limits<T>::max();
    std::vector<T> dist(n, INF);
    std::vector<int> parent(n, -1);

    DHeap<T> heap(n);

    dist[start] = T();
    heap.push(start, T());

    while (!heap.empty()) {
        int v = heap.popMin();

        for (size_t j = 0; j < graph[v].size(); ++j) {
            const std::pair<int, T>& edge = graph[v][j];
            int to = edge.first;
            T weight = edge.second;

            if (dist[v] != INF && dist[v] + weight < dist[to]) {
                dist[to] = dist[v] + weight;
                parent[to] = v;
                heap.push(to, dist[to]);
            }
        }
    }

    return dist;
}

// ==================== TESTS ====================

// Тесты для DHeap
TEST(DHeapTest, PushAndPopMin) {
    DHeap<int> heap(10);
    heap.push(0, 5);
    heap.push(1, 3);
    heap.push(2, 7);

    EXPECT_EQ(heap.popMin(), 1);
    EXPECT_EQ(heap.popMin(), 0);
    EXPECT_EQ(heap.popMin(), 2);
    EXPECT_TRUE(heap.empty());
}

TEST(DHeapTest, DecreaseKey) {
    DHeap<int> heap(10);
    heap.push(0, 10);
    heap.push(1, 20);
    heap.push(2, 30);

    heap.decreaseKey(2, 5);
    EXPECT_EQ(heap.popMin(), 2);
    EXPECT_EQ(heap.popMin(), 0);
    EXPECT_EQ(heap.popMin(), 1);
}

TEST(DHeapTest, ContainsVertex) {
    DHeap<int> heap(10);
    heap.push(0, 5);
    heap.push(1, 3);

    EXPECT_TRUE(heap.contains(0));
    EXPECT_TRUE(heap.contains(1));
    EXPECT_FALSE(heap.contains(2));

    heap.popMin();
    EXPECT_FALSE(heap.contains(1));
    EXPECT_TRUE(heap.contains(0));
}

TEST(DHeapTest, PushExistingVertex) {
    DHeap<int> heap(10);
    heap.push(0, 10);
    heap.push(0, 5);

    EXPECT_EQ(heap.getKey(0), 5);
    EXPECT_EQ(heap.size(), 1);
}

TEST(DHeapTest, DifferentDValues) {
    DHeap<int, 2> binaryHeap(10);
    binaryHeap.push(0, 10);
    binaryHeap.push(1, 5);
    binaryHeap.push(2, 8);

    EXPECT_EQ(binaryHeap.popMin(), 1);
    EXPECT_EQ(binaryHeap.popMin(), 2);
    EXPECT_EQ(binaryHeap.popMin(), 0);

    DHeap<int, 8> wideHeap(10);
    wideHeap.push(0, 10);
    wideHeap.push(1, 5);
    wideHeap.push(2, 8);

    EXPECT_EQ(wideHeap.popMin(), 1);
    EXPECT_EQ(wideHeap.popMin(), 2);
    EXPECT_EQ(wideHeap.popMin(), 0);
}

TEST(DHeapTest, EmptyHeapPopThrows) {
    DHeap<int> heap(10);
    EXPECT_THROW(heap.popMin(), std::runtime_error);
}

TEST(DHeapTest, GetKeyFromNonExistentVertexThrows) {
    DHeap<int> heap(10);
    heap.push(0, 5);
    EXPECT_THROW(heap.getKey(1), std::runtime_error);
}

// Тесты для Дейкстры
TEST(DijkstraTest, SimpleGraphInt) {
    std::vector<std::vector<std::pair<int, int>>> graph(4);
    graph[0].push_back(std::make_pair(1, 1));
    graph[0].push_back(std::make_pair(2, 4));
    graph[1].push_back(std::make_pair(2, 2));
    graph[1].push_back(std::make_pair(3, 5));
    graph[2].push_back(std::make_pair(3, 1));

    std::vector<int> dist = dijkstra(0, graph);

    EXPECT_EQ(dist[0], 0);
    EXPECT_EQ(dist[1], 1);
    EXPECT_EQ(dist[2], 3);
    EXPECT_EQ(dist[3], 4);
}

TEST(DijkstraTest, DisconnectedGraph) {
    std::vector<std::vector<std::pair<int, int>>> graph(4);
    graph[0].push_back(std::make_pair(1, 1));
    graph[1].push_back(std::make_pair(0, 1));
    graph[2].push_back(std::make_pair(3, 1));
    graph[3].push_back(std::make_pair(2, 1));

    std::vector<int> dist = dijkstra(0, graph);

    EXPECT_EQ(dist[0], 0);
    EXPECT_EQ(dist[1], 1);
    EXPECT_EQ(dist[2], std::numeric_limits<int>::max());
    EXPECT_EQ(dist[3], std::numeric_limits<int>::max());
}


TEST(DijkstraTest, FromExample) {
    std::vector<std::vector<std::pair<int, int>>> graph(6);
    graph[0].push_back(std::make_pair(1, 7));
    graph[0].push_back(std::make_pair(2, 9));
    graph[0].push_back(std::make_pair(5, 14));
    graph[1].push_back(std::make_pair(2, 10));
    graph[1].push_back(std::make_pair(3, 15));
    graph[2].push_back(std::make_pair(3, 11));
    graph[2].push_back(std::make_pair(5, 2));
    graph[3].push_back(std::make_pair(4, 6));
    graph[4].push_back(std::make_pair(5, 9));
    graph[5].push_back(std::make_pair(4, 9));

    std::vector<int> dist = dijkstra(0, graph);

    EXPECT_EQ(dist[0], 0);
    EXPECT_EQ(dist[1], 7);
    EXPECT_EQ(dist[2], 9);
    EXPECT_EQ(dist[3], 20);
    EXPECT_EQ(dist[4], 20);
    EXPECT_EQ(dist[5], 11);
}

TEST(DijkstraTest, SingleVertex) {
    std::vector<std::vector<std::pair<int, int>>> graph(1);
    std::vector<int> dist = dijkstra(0, graph);

    EXPECT_EQ(dist[0], 0);
}

TEST(DijkstraTest, CompleteGraph) {
    const int N = 5;
    std::vector<std::vector<std::pair<int, int>>> graph(N);

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i != j) {
                graph[i].push_back(std::make_pair(j, 1));
            }
        }
    }

    std::vector<int> dist = dijkstra(0, graph);

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(dist[i], (i == 0 ? 0 : 1));
    }
}

TEST(DijkstraTest, StartFromNonZeroVertex) {
    std::vector<std::vector<std::pair<int, int>>> graph(4);
    graph[0].push_back(std::make_pair(1, 1));
    graph[1].push_back(std::make_pair(2, 2));
    graph[2].push_back(std::make_pair(3, 3));

    std::vector<int> dist = dijkstra(1, graph);

    EXPECT_EQ(dist[0], std::numeric_limits<int>::max());
    EXPECT_EQ(dist[1], 0);
    EXPECT_EQ(dist[2], 2);
    EXPECT_EQ(dist[3], 5);
}

TEST(DijkstraTest, ShortPathWithEqualWeights) {
    std::vector<std::vector<std::pair<int, int>>> graph(4);
    graph[0].push_back(std::make_pair(1, 2));
    graph[0].push_back(std::make_pair(2, 2));
    graph[1].push_back(std::make_pair(3, 2));
    graph[2].push_back(std::make_pair(3, 2));

    std::vector<int> dist = dijkstra(0, graph);

    EXPECT_EQ(dist[0], 0);
    EXPECT_EQ(dist[1], 2);
    EXPECT_EQ(dist[2], 2);
    EXPECT_EQ(dist[3], 4);
}


int main(int argc, char** argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}