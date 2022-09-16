template<typename T>
void construct(T obj, )


template<typename T>
void construct(T obj) {
    __construct(obj, value_type(obj));
}