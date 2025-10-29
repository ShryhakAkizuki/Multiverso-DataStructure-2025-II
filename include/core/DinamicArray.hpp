#pragma once
#include <memory>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

template<typename T>
class DinamicArray {
private:
    // ----- Atributos -----
    uint32_t _capacity = 0;
    uint32_t _size = 0;
    std::unique_ptr<T[]> _data = nullptr;

public:
    // ----- Constructores -----
    DinamicArray() = default;                       // Constructor por defecto

    explicit DinamicArray(uint32_t capacity) :      // Constructor con capacidad inicial
    _capacity(capacity), 
    _size(0) {
        _data = std::make_unique<T[]>(_capacity);
    }

    DinamicArray(std::initializer_list<T> init) :   // Constructor con lista de inicialización = {1, 2, ...}
    _capacity(static_cast<uint32_t>(init.size())), 
    _size(static_cast<uint32_t>(init.size())) {
        
        _data = std::make_unique<T[]>(_capacity);
        uint32_t i = 0;
        for (const auto& item : init) {
            _data[i++] = item;
        }
    }

    DinamicArray(const DinamicArray& other) :       // Constructor de copia
    _capacity(other._capacity), 
    _size(other._size) {
        
        _data = std::make_unique<T[]>(_capacity);
        for (uint32_t i = 0; i < _size; ++i) {
            _data[i] = other._data[i];
        }
    }

    DinamicArray(DinamicArray&& other) noexcept :   // Constructor de movimiento
        _capacity(other._capacity),
        _size(other._size),
        _data(std::move(other._data)) {
        
        other._capacity = 0;
        other._size = 0;
    }

    // ----- Destructor -----
    ~DinamicArray() = default;

    // ----- Operadores -----
    DinamicArray& operator=(const DinamicArray& other) {        // Operador de asignación por copia
        if(this != &other) {
            _capacity = other._capacity;
            _size = other._size;
            _data = std::make_unique<T[]>(_capacity);
            for (uint32_t i = 0; i < _size; ++i) {
                _data[i] = other._data[i];
            }
        }
        return *this;
    }

    DinamicArray& operator=(DinamicArray&& other) noexcept {    // Operador de asignación por movimiento
        if(this != &other) {
            _capacity = other._capacity;
            _size = other._size;
            _data = std::move(other._data);

            other._capacity = 0;
            other._size = 0;
        }
        return *this;
    }

    T& operator[](uint32_t index) {                             // Operador de acceso
        if( index >= _size) {
            throw std::out_of_range("Indice fuera de rango");
        }
        return _data[index];
    }
        
    const T& operator[](uint32_t index) const {                 // Operador de acceso constanter
        if( index >= _size) {
            throw std::out_of_range("Indice fuera de rango");
        }
        return _data[index];
    }

    // ----- Métodos -----

    // Capacidad
    bool empty() const noexcept {           // Booleano - Arreglo vacio
        return _size == 0;
    };

    bool full() const noexcept {            // Booleano - Arreglo lleno
        return _size == _capacity;
    }

    uint32_t size() const noexcept {        // Retornar el tamaño actual del arreglo
        return _size;
    }

    uint32_t capacity() const noexcept {    // Retornar el tamaño reservado del arreglo
        return _capacity;
    }

    void reserve(uint32_t new_capacity) {   // Reserva una determinada cantidad de memoria
        if(new_capacity> _capacity){
            resize(new_capacity);
        }
    }

    void shrink_to_fit(){                   // Delimita el tamaño del arreglo al tamaño justo
        if(_size < _capacity){
            resize(_size);
        }
    }

    // Asignacion y retorno
    void clear() noexcept{                  // Limpia la lista
        _size = 0;
    }

    void push_back(const T& value){         // Ingresa el valor al final de la lista
        if(_size >= _capacity){
            reserve(_capacity == 0 ? 4: _capacity * 2);
        }
        _data[_size++] = value;
    }

    void push_back(T&& value){              // Ingresa el valor al final de la lista con move semantics
        if(_size >= _capacity){
            reserve(_capacity == 0 ? 4: _capacity * 2);
        }
        _data[_size++] = std::move(value);
    }

    T pop_back(){                           // Retira el ultimo valor de la lista 
        if(_size < 1){
            throw std::out_of_range("No hay elementos para retirar del arreglo");
        }

        if(_size <= _capacity/4){
            shrink_to_fit();
        }
        T result = std::move(_data[--_size]); 
        return result;
    }

    T& front() {                            // Acceso primer valor
        if (_size == 0) throw std::out_of_range("Array is empty");
        return _data[0];
    }
    
    const T& front() const {                // Copia primer valor
        if (_size == 0) throw std::out_of_range("Array is empty");
        return _data[0];
    }
    
    T& back() {                             // Acceso ultimo valor
        if (_size == 0) throw std::out_of_range("Array is empty");
        return _data[_size - 1];
    }
    
    const T& back() const {                 // Copia ultimo valor
        if (_size == 0) throw std::out_of_range("Array is empty");
        return _data[_size - 1];
    }

    T* data() noexcept {
        return _data.get();
    }
    
    const T* data() const noexcept {
        return _data.get();
    }

    // Iteradores
    T* begin() noexcept {
        return _data.get();
    }

    const T* begin() const noexcept {
        return _data.get();
    }

    T* end() noexcept {
        return _data.get() + _size;
    }
    
    const T* end() const noexcept {
        return _data.get() + _size;
    }

private:
    // ----- Redimensionamiento -----
    void resize(uint32_t new_capacity) {
        auto new_data = std::make_unique<T[]>(new_capacity);

        for (uint32_t i = 0; i < _size; ++i){
            new_data[i] = std::move(_data[i]);
        }

        _data = std::move(new_data);
        _capacity = new_capacity;
    }

};