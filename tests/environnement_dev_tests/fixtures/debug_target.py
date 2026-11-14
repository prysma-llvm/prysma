def compute_double(value):
    doubled = value * 2
    return doubled


def main():
    int_var = 42
    bool_var = True
    float_var = 3.14159
    str_var = "prysma_compiler"
    unicode_var = "éàçü_テスト"
    empty_str = ""
    list_var = [10, 20, 30, 40]
    result = compute_double(int_var)
    print(f"Calculated: {result}")


if __name__ == "__main__":
    main()
