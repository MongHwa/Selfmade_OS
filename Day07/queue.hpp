#include <cstdint>
#include <array>

template<typename T>
class ArrayQueue {
    public:
        template <size_t N>
        ArrayQueue(std::array<T, N>& buf);
        ArrayQueue(T* buf, size_t size);
        Error Push(const T& value);
        Error Pop();
        size_t Count() const;
        size_t Capacity() const;
        const T& Front() const;
    
    private:
        T* data_;
        size_t read_pos_, write_pos_, count_;
        //현재 front 위치, 다음 데이터 삽입 위치, 현재 큐에 들어있는 요소 개수
        const size_t capacity_;
};

template<typename T>
template<size_t N>
ArrayQueue<T>::ArrayQueue(std::array<T, N>& buf) : ArrayQueue(buf.data(), N) {}
//파라미터 2개를 받는 생성자에 이양

template<typename T>
ArrayQueue<T>::ArrayQueue(T* buf, size_t size) 
    : data_{buf}, read_pos_{0}, write_pos_{0}, count_{0}, capacity_{size} {}

template<typename T>
Error ArrayQueue<T>::Push(const T& value) {
    if(count == capacity_) {
        return MAKE_ERROR(Error::kFull);
    }

    data_[wrtie_pos_] = value;
    write_pos_++;
    count__++;
    if(write_pos_ == capacity_) {
        write_pos_ = 0;
    }

    return MAKE_ERROR(Error::kSuccess);
}

template<typename T>
Error ArrayQueue<T>::Pop() {
    if(count_ == 0) {
        return MAKE_ERROR(Error::kEmpty);
    }

    count_--;
    read_pos_++;
    if(read_pos_ == capacity_) {
        read_pos_ = 0;
    }

    return MAKE_ERROR(Error::kSuccess);
}

template<typename T>
const T& ArrayQueue<T>::Front() const {
    return data_[read_pos_];
}