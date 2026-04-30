
// Python like random number generator for C++
class Random {
public:

    Random() = default;

    // Returns a random float between 0 and 1
    float random() {
        return static_cast<float>( rand() ) / static_cast<float>( RAND_MAX );
    }    

    float random(float min, float max) {
        return min + random() * (max - min);
    }

    int random(int min, int max) {
        return min + static_cast<int>( random() * (max - min + 1) );
    }
};