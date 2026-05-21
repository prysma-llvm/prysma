//===-- registry_generic.h --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef E962F97C_7F79_445E_9F65_097981CC00B4
#define E962F97C_7F79_445E_9F65_097981CC00B4

#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <functional>
#include <sstream>
#include <mutex>
#include <type_traits>
#include <llvm/ADT/StringRef.h>

// Phantom structure for local (single-threaded) usage
struct RegistryLock {
    void lock() const{}
    void unlock() const {}
};

template <typename TKey, typename TValue, typename TLock = RegistryLock>
class RegistryGeneric {
private:
    std::map<TKey, TValue> _elements;
    std::function<std::string(TKey)> _errorMessageCallback;
    mutable TLock _lock;
public:
    RegistryGeneric() = default;
    virtual ~RegistryGeneric() = default;

    RegistryGeneric(const RegistryGeneric&) = delete;
    auto operator=(const RegistryGeneric&) -> RegistryGeneric& = delete;
    RegistryGeneric(RegistryGeneric&&) = delete;
    auto operator=(RegistryGeneric&&) -> RegistryGeneric& = delete;
   
    void setErrorMessage(std::function<std::string(TKey)>&& callback) {
        std::lock_guard<TLock> guard(_lock);
        _errorMessageCallback = std::move(callback);
    }

    void registerElement(const TKey& key, TValue value) {
        std::lock_guard<TLock> guard(_lock);
            std::cout << "Registering element in RegistryGeneric for " << " : " << typeid(TKey).name() << "\n";

            std::cout << "\t -> generic reg addr: " << this << "\n";
            std::cout << "\t -> map addr: " << &_elements << "\n";


        _elements[key] = std::move(value);
    }

    auto get(const TKey& key) const -> const TValue& {
        std::lock_guard<TLock> guard(_lock);
        auto iterator = _elements.find(key);
        std::cout << "calling get from RegistryGeneric for " << " : " << typeid(TKey).name() << "\n";

        std::cout << "\t -> generic reg addr: " << this << "\n";
        std::cout << "\t -> map addr: " << &_elements << "\n";

        std::cout << "map size : " << _elements.size() << "\n";


        if (iterator == _elements.end()) {
            throw std::invalid_argument(generateInternalErrorMessage(key));
        }
        return iterator->second;
    }

    auto exists(const TKey& key) const -> bool {
        std::lock_guard<TLock> guard(_lock);
        return _elements.count(key) > 0;
    }

    auto getKeys() const -> std::set<TKey> {
        std::lock_guard<TLock> guard(_lock);
        std::set<TKey> keys;
        for (const auto& pair : _elements) {
            keys.insert(pair.first);
        }
        return keys;
    }

protected:
    virtual auto generateErrorMessage(const TKey& key) const -> std::string {
        std::lock_guard<TLock> guard(_lock);
        return generateInternalErrorMessage(key);
    }

private:
    auto generateInternalErrorMessage(const TKey& key) const -> std::string {
        std::cout << "\t ERROR -> generic reg addr: " << this << "\n";
        std::cout << "\t ERROR -> map addr: " << &_elements << "\n";

        if (_errorMessageCallback) {
            return _errorMessageCallback(key);
        }
        std::stringstream stringStream;
        if constexpr (std::is_same_v<TKey, llvm::StringRef>) {
            stringStream << "Unknown element in registry: " << key.str();
        } else {
            stringStream << "Unknown element in registry: " << key;
        }
        return stringStream.str();
    }
};

#endif
