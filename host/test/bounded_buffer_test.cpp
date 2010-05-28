//
// Copyright 2010 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <boost/test/unit_test.hpp>
#include <uhd/transport/bounded_buffer.hpp>

using namespace uhd::transport;

static const boost::posix_time::milliseconds timeout(10);

BOOST_AUTO_TEST_CASE(test_bounded_buffer_with_timed_wait){
    bounded_buffer<int>::sptr bb(new bounded_buffer<int>(3));

    //push elements, check for timeout
    BOOST_CHECK(bb->push_with_timed_wait(0, timeout));
    BOOST_CHECK(bb->push_with_timed_wait(1, timeout));
    BOOST_CHECK(bb->push_with_timed_wait(2, timeout));
    BOOST_CHECK(not bb->push_with_timed_wait(3, timeout));

    int val;
    //pop elements, check for timeout and check values
    BOOST_CHECK(bb->pop_with_timed_wait(val, timeout));
    BOOST_CHECK_EQUAL(val, 0);
    BOOST_CHECK(bb->pop_with_timed_wait(val, timeout));
    BOOST_CHECK_EQUAL(val, 1);
    BOOST_CHECK(bb->pop_with_timed_wait(val, timeout));
    BOOST_CHECK_EQUAL(val, 2);
    BOOST_CHECK(not bb->pop_with_timed_wait(val, timeout));
}

BOOST_AUTO_TEST_CASE(test_bounded_buffer_with_pop_on_full){
    bounded_buffer<int>::sptr bb(new bounded_buffer<int>(3));

    //push elements, check for timeout
    BOOST_CHECK(bb->push_with_pop_on_full(0));
    BOOST_CHECK(bb->push_with_pop_on_full(1));
    BOOST_CHECK(bb->push_with_pop_on_full(2));
    BOOST_CHECK(not bb->push_with_pop_on_full(3));

    int val;
    //pop elements, check for timeout and check values
    BOOST_CHECK(bb->pop_with_timed_wait(val, timeout));
    BOOST_CHECK_EQUAL(val, 1);
    BOOST_CHECK(bb->pop_with_timed_wait(val, timeout));
    BOOST_CHECK_EQUAL(val, 2);
    BOOST_CHECK(bb->pop_with_timed_wait(val, timeout));
    BOOST_CHECK_EQUAL(val, 3);
}

#include <uhd/transport/alignment_buffer.hpp>
#include <boost/assign/list_of.hpp>

using namespace boost::assign;

BOOST_AUTO_TEST_CASE(test_alignment_buffer_tmp){
    alignment_buffer<int, size_t>::sptr ab(new alignment_buffer<int, size_t>(5, 3));
    //load index 0 with all good seq numbers
    BOOST_CHECK(ab->push_with_pop_on_full(0, 0, 0));
    BOOST_CHECK(ab->push_with_pop_on_full(1, 1, 0));
    BOOST_CHECK(ab->push_with_pop_on_full(2, 2, 0));
    BOOST_CHECK(ab->push_with_pop_on_full(3, 3, 0));
    BOOST_CHECK(ab->push_with_pop_on_full(4, 4, 0));

    //load index 1 with some skipped seq numbers
    BOOST_CHECK(ab->push_with_pop_on_full(10, 0, 1));
    BOOST_CHECK(ab->push_with_pop_on_full(11, 1, 1));
    BOOST_CHECK(ab->push_with_pop_on_full(14, 4, 1));
    BOOST_CHECK(ab->push_with_pop_on_full(15, 5, 1));
    BOOST_CHECK(ab->push_with_pop_on_full(16, 6, 1));

    //load index 2 with all good seq numbers
    BOOST_CHECK(ab->push_with_pop_on_full(20, 0, 2));
    BOOST_CHECK(ab->push_with_pop_on_full(21, 1, 2));
    BOOST_CHECK(ab->push_with_pop_on_full(22, 2, 2));
    BOOST_CHECK(ab->push_with_pop_on_full(23, 3, 2));
    BOOST_CHECK(ab->push_with_pop_on_full(24, 4, 2));

    //readback aligned values
    std::vector<int> aligned_elems(3);

    std::vector<int> expected_elems0 = list_of(0)(10)(20);
    BOOST_CHECK(ab->pop_elems_with_timed_wait(aligned_elems, timeout));
    BOOST_CHECK_EQUAL_COLLECTIONS(
        aligned_elems.begin(), aligned_elems.end(),
        expected_elems0.begin(), expected_elems0.end()
    );

    std::vector<int> expected_elems1 = list_of(1)(11)(21);
    BOOST_CHECK(ab->pop_elems_with_timed_wait(aligned_elems, timeout));
    BOOST_CHECK_EQUAL_COLLECTIONS(
        aligned_elems.begin(), aligned_elems.end(),
        expected_elems1.begin(), expected_elems1.end()
    );

    //there was a skip now find 4

    std::vector<int> expected_elems4 = list_of(4)(14)(24);
    BOOST_CHECK(ab->pop_elems_with_timed_wait(aligned_elems, timeout));
    BOOST_CHECK_EQUAL_COLLECTIONS(
        aligned_elems.begin(), aligned_elems.end(),
        expected_elems4.begin(), expected_elems4.end()
    );
}