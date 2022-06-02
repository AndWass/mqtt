//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/container/pmr/vector.hpp>
#include <boost/core/span.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>

namespace purple {
class binary {
public:
    using container_type = boost::container::pmr::vector<uint8_t>;

private:
    boost::shared_ptr<container_type> storage_;

    void ensure_unique() {
        if (unique()) {
            return;
        }
        *this = deep_clone();
    }

    struct assign_ptr_tag {};

    binary(assign_ptr_tag, boost::shared_ptr<container_type> storage) : storage_(std::move(storage)) {
    }

public:
    using value_type = uint8_t;
    using reference = uint8_t &;
    using const_reference = const uint8_t &;
    using size_type = boost::container::pmr::vector<uint8_t>::size_type;
    using allocator_type = boost::container::pmr::memory_resource *;
    using iterator = boost::container::pmr::vector<uint8_t>::const_iterator;
    using const_iterator = iterator;

    explicit binary(allocator_type alloc = nullptr) : storage_(boost::make_shared<container_type>(alloc)) {
    }

    binary(const binary &other, allocator_type alloc) : storage_(std::move(other.deep_clone(alloc).storage_)) {
    }

    template<class InputIt>
    binary(InputIt begin, InputIt end, allocator_type alloc = nullptr)
        : storage_(boost::make_shared<container_type>(begin, end, alloc)) {
    }

    template<class Vector, std::enable_if_t<std::is_same<std::decay_t<Vector>, container_type>::value> * = nullptr>
    explicit binary(Vector &&vec)
        : storage_(boost::make_shared<container_type>(std::forward<Vector>(vec), vec.get_allocator())) {
    }

    binary(boost::span<const uint8_t> data, allocator_type alloc = nullptr) : binary(data.begin(), data.end(), alloc) {
    }

    binary(boost::span<uint8_t> data, allocator_type alloc = nullptr) : binary(data.begin(), data.end(), alloc) {
    }

    binary(boost::string_view data, allocator_type alloc = nullptr) : binary(data.begin(), data.end(), alloc) {
    }

    void assign(size_type count, uint8_t value) {
        ensure_unique();
        storage_->assign(count, value);
    }

    template<class InputIt, std::enable_if_t<!std::is_convertible_v<InputIt, size_type>> * = nullptr>
    void assign(InputIt first, InputIt last) {
        ensure_unique();
        storage_->assign(first, last);
    }

    const_iterator begin() const {
        return storage_->cbegin();
    }

    const_iterator end() const {
        return storage_->cend();
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return cend();
    }

    const uint8_t *data() const {
        return storage_->data();
    }

    uint8_t *data_mut() {
        ensure_unique();
        return storage_->data();
    }

    size_type size() const {
        return storage_->size();
    }

    bool empty() const {
        return storage_->empty();
    }

    const_iterator insert(const const_iterator &pos, uint8_t data) {
        ensure_unique();
        return storage_->insert(pos, data);
    }

    const_iterator insert(const const_iterator &pos, size_type count, uint8_t data) {
        ensure_unique();
        return storage_->insert(pos, count, data);
    }

    template<class InputIt, std::enable_if_t<!std::is_convertible_v<InputIt, size_type>> * = nullptr>
    const_iterator insert(const const_iterator &pos, InputIt first, InputIt last) {
        ensure_unique();
        return storage_->insert(pos, first, last);
    }

    void push_back(value_type data) {
        ensure_unique();
        storage_->push_back(data);
    }

    void pop_back(value_type data) {
        ensure_unique();
        storage_->pop_back();
    }

    const_reference front() const {
        return storage_->front();
    }

    const_reference back() const {
        return storage_->back();
    }

    reference front_mut() {
        ensure_unique();
        return storage_->front();
    }

    reference back_mut() {
        ensure_unique();
        return storage_->back();
    }

    const_reference operator[](size_t idx) const {
        return (*storage_)[idx];
    }

    const_reference at(size_t idx) const {
        return storage_->at(idx);
    }

    reference at_mut(size_t idx) {
        ensure_unique();
        return storage_->at(idx);
    }

    reference at_mut_unchecked(size_t idx) {
        ensure_unique();
        return (*storage_)[idx];
    }

    container_type &mut_ref() {
        ensure_unique();
        return *storage_;
    }

    bool unique() const {
        return storage_.unique();
    }

    binary deep_clone() const {
        return binary(assign_ptr_tag{}, boost::make_shared<container_type>(*storage_, storage_->get_allocator()));
    }

    binary deep_clone(container_type::allocator_type alloc) const {
        return binary(assign_ptr_tag{}, boost::make_shared<container_type>(*storage_, alloc));
    }
};

class binary_view : public boost::span<const uint8_t> {
public:
    using base_type = boost::span<const uint8_t>;

    using base_type::span;
    binary_view(const char *str)// NOLINT(google-explicit-constructor)
        : boost::span<const uint8_t>(reinterpret_cast<const uint8_t *>(str), strlen(str)) {
    }
    binary_view(const char *str, size_t len) : boost::span<const uint8_t>(reinterpret_cast<const uint8_t *>(str), len) {
    }
    binary_view(boost::string_view str)// NOLINT(google-explicit-constructor)
        : boost::span<const uint8_t>(reinterpret_cast<const uint8_t *>(str.data()), str.size()) {
    }
};
}// namespace purple