///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl

Copyright(c) 2014 jwellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef __ETL_IBITSET__
#define __ETL_IBITSET__

#include <algorithm>
#include <stdint.h>

#include "exception.h"
#include "integral_limits.h"
#include "binary.h"

#if WIN32
#undef min
#endif

namespace etl
{
  //***************************************************************************
  /// Exception base for bitset
  ///\ingroup bitset
  //***************************************************************************
  class bitset_exception : public etl::exception
  {
  public:

    bitset_exception(string_type what, string_type file_name, numeric_type line_number)
      : exception(what, file_name, line_number)
    {
    }
  };

  //***************************************************************************
  /// Bitset nullptr exception.
  ///\ingroup bitset
  //***************************************************************************
  class bitset_nullptr : public bitset_exception
  {
  public:

    bitset_nullptr(string_type file_name, numeric_type line_number)
      : bitset_exception("bitset: nullptr", file_name, line_number)
    {
    }
  };

  //*************************************************************************
  /// The base class for etl::bitset
  ///\ingroup bitset
  //*************************************************************************
  class ibitset
  {
  protected:

    // The type used for each element in the array.
    typedef uint8_t element_t;

  public:

    static const element_t ALL_SET   = etl::integral_limits<element_t>::max;
    static const element_t ALL_CLEAR = 0;

    static const size_t    BITS_PER_ELEMENT = etl::integral_limits<element_t>::bits;

    enum
    {
      npos = etl::integral_limits<size_t>::max
    };

    //*************************************************************************
    /// The reference type returned.
    //*************************************************************************
    class bit_reference
    {
    public:

      friend class ibitset;

      //*******************************
      /// Conversion operator.
      //*******************************
      operator bool() const
      {
        return p_bitset->test(position);
      }

      //*******************************
      /// Assignment operator.
      //*******************************
      bit_reference& operator = (bool b)
      {
        p_bitset->set(position, b);
        return *this;
      }

      //*******************************
      /// Assignment operator.
      //*******************************
      bit_reference& operator = (const bit_reference& r)
      {
        p_bitset->set(position, bool(r));
        return *this;
      }

      //*******************************
      /// Flip the bit.
      //*******************************
      bit_reference& flip()
      {
        p_bitset->flip(position);
        return *this;
      }

      //*******************************
      /// Return the logical inverse of the bit.
      //*******************************
      bool operator~() const
      {
        return !p_bitset->test(position);
      }

    private:

      //*******************************
      /// Default constructor.
      //*******************************
      bit_reference()
        : p_bitset(nullptr),
        position(0)
      {
      }

      //*******************************
      /// Constructor.
      //*******************************
      bit_reference(ibitset& r_bitset, size_t position)
        : p_bitset(&r_bitset),
          position(position)
      {
      }

      ibitset* p_bitset; ///< The bitset.
      size_t   position; ///< The position in the bitset.
    };

    //*************************************************************************
    /// The size of the bitset.
    //*************************************************************************
    size_t size() const
    {
      return NBITS;
    }

    //*************************************************************************
    /// Count the number of bits set.
    //*************************************************************************
    size_t count() const
    {
      size_t n = 0;

      for (size_t i = 0; i < SIZE; ++i)
      {
        n += etl::count_bits(pdata[i]);
      }

      return n;
    }

    //*************************************************************************
    /// Tests a bit at a position.
    /// Positions greater than the number of configured bits will return <b>false</b>.
    //*************************************************************************
    bool test(size_t position) const
    {
      size_t       index;
      element_t mask;

      if (SIZE == 1)
      {
        index = 0;
        mask = element_t(1) << position;
      }
      else
      {
        index = position >> etl::log2<BITS_PER_ELEMENT>::value;
        mask = element_t(1) << (position & (BITS_PER_ELEMENT - 1));
      }

      return (pdata[index] & mask) != 0;
    }

    //*************************************************************************
    /// Set the bit at the position.
    //*************************************************************************
    ibitset& set()
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        pdata[i] = ALL_SET;
      }

      pdata[SIZE - 1] &= TOP_MASK;

      return *this;
    }

    //*************************************************************************
    /// Set the bit at the position.
    //*************************************************************************
    ibitset& set(size_t position, bool value = true)
    {
      size_t    index;
      element_t bit;

      if (SIZE == 1)
      {
        index = 0;
        bit   = element_t(1) << position;
      }
      else
      {
        index = position >> etl::log2<BITS_PER_ELEMENT>::value;
        bit   = element_t(1) << (position & (BITS_PER_ELEMENT - 1));
      }

      if (value)
      {
        pdata[index] |= bit;
      }
      else
      {
        pdata[index] &= ~bit;
      }

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    ibitset& set(const char* text)
    {
      reset();

      size_t i = std::min(NBITS, strlen(text));

      while (i > 0)
      {
        set(--i, *text++ == '1');
      }

      return *this;
    }

    //*************************************************************************
    /// Resets the bitset.
    //*************************************************************************
    ibitset& reset()
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        pdata[i] = ALL_CLEAR;
      }

      return *this;
    }

    //*************************************************************************
    /// Reset the bit at the position.
    //*************************************************************************
    ibitset& reset(size_t position)
    {
      size_t       index;
      element_t bit;

      if (SIZE == 1)
      {
        index = 0;
        bit   = element_t(1) << position;
      }
      else
      {
        index = position >> etl::log2<BITS_PER_ELEMENT>::value;
        bit   = element_t(1) << (position & (BITS_PER_ELEMENT - 1));
      }

      pdata[index] &= ~bit;

      return *this;
    }

    //*************************************************************************
    /// Flip all of the bits.
    //*************************************************************************
    ibitset& flip()
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        pdata[i] = ~pdata[i];
      }

      pdata[SIZE - 1] &= TOP_MASK;

      return *this;
    }

    //*************************************************************************
    /// Flip the bit at the position.
    //*************************************************************************
    ibitset& flip(size_t position)
    {
      if (position < NBITS)
      {
        size_t    index;
        element_t bit;

        if (SIZE == 1)
        {
          index = 0;
          bit   = element_t(1) << position;
        }
        else
        {
          index = position >> log2<BITS_PER_ELEMENT>::value;
          bit   = element_t(1) << (position & (BITS_PER_ELEMENT - 1));
        }

        pdata[index] ^= bit;
      }

      return *this;
    }

    //*************************************************************************
    // Are all the bits sets?
    //*************************************************************************
    bool all() const
    {
      // All but the last.
      for (size_t i = 0; i < (SIZE - 1); ++i)
      {
        if (pdata[i] != ALL_SET)
        {
          return false;
        }
      }

      // The last.
      if (pdata[SIZE - 1] != (ALL_SET & TOP_MASK))
      {
        return false;
      }

      return true;
    }

    //*************************************************************************
    /// Are any of the bits set?
    //*************************************************************************
    bool any() const
    {
      return !none();
    }

    //*************************************************************************
    /// Are none of the bits set?
    //*************************************************************************
    bool none() const
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        if (pdata[i] != 0)
        {
          return false;
        }
      }

      return true;
    }

    //*************************************************************************
    /// Finds the first bit in the specified state.
    ///\param state The state to search for.
    ///\returns The position of the bit or SIZE if none were found.
    //*************************************************************************
    size_t find_first(bool state) const
    {
      return find_next(state, 0);
    }

    //*************************************************************************
    /// Finds the next bit in the specified state.
    ///\param state    The state to search for.
    ///\param position The position to start from.
    ///\returns The position of the bit or SIZE if none were found.
    //*************************************************************************
    size_t find_next(bool state, size_t position) const
    {
      // Where to start.
      size_t index;
      size_t bit;

      if (SIZE == 1)
      {
        index = 0;
        bit   = position;
      }
      else
      {
        index = position >> log2<BITS_PER_ELEMENT>::value;
        bit   = position & (BITS_PER_ELEMENT - 1);
      }

      element_t mask = 1 << bit;

      // For each element in the bitset...
      while (index < SIZE)
      {
        element_t value = pdata[index];

        // Needs checking?
        if (( state && (value != ALL_CLEAR)) ||
            (!state && (value != ALL_SET)))
        {
          // For each bit in the element...
          while ((bit < BITS_PER_ELEMENT) && (position < NBITS))
          {
            // Equal to the required state?
            if (((value & mask) != 0) == state)
            {
              return position;
            }

            // Move on to the next bit.
            mask <<= 1;
            ++position;
            ++bit;
          }
        }
        else
        {
          position += BITS_PER_ELEMENT;
        }

        // Start at the beginning for all other elements.
        bit  = 0;
        mask = 1;

        ++index;
      }

      return ibitset::npos;
    }

    //*************************************************************************
    /// Read [] operator.
    //*************************************************************************
    bool operator[] (size_t position) const
    {
      return test(position);
    }

    //*************************************************************************
    /// Write [] operator.
    //*************************************************************************
    bit_reference operator [] (size_t position)
    {
      return bit_reference(*this, position);
    }

    //*************************************************************************
    /// operator &=
    //*************************************************************************
    ibitset& operator &=(const ibitset& other)
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        pdata[i] &= other.pdata[i];
      }

      return *this;
    }

    //*************************************************************************
    /// operator |=
    //*************************************************************************
    ibitset& operator |=(const ibitset& other)
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        pdata[i] |= other.pdata[i];
      }

      return *this;
    }

    //*************************************************************************
    /// operator ^=
    //*************************************************************************
    ibitset& operator ^=(const ibitset& other)
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        pdata[i] ^= other.pdata[i];
      }

      return *this;
    }

    //*************************************************************************
    /// operator <<=
    //*************************************************************************
    ibitset& operator<<=(size_t shift)
    {
      if (SIZE == 1)
      {
        pdata[0] <<= shift;
      }
      else
      {
        size_t source      = NBITS - shift - 1;
        size_t destination = NBITS - 1;

        for (size_t i = 0; i < (NBITS - shift); ++i)
        {
          set(destination--, test(source--));
        }

        for (size_t i = 0; i < shift; ++i)
        {
          reset(destination--);
        }
      }

      return *this;
    }

    //*************************************************************************
    /// operator >>=
    //*************************************************************************
    ibitset& operator>>=(size_t shift)
    {
      if (SIZE == 1)
      {
        pdata[0] >>= shift;
      }
      else
      {
        size_t source      = shift;
        size_t destination = 0;

        for (size_t i = 0; i < (NBITS - shift); ++i)
        {
          set(destination++, test(source++));
        }

        for (size_t i = 0; i < shift; ++i)
        {
          reset(destination++);
        }
      }

      return *this;
    }

    //*************************************************************************
    /// swap
    //*************************************************************************
    void swap(ibitset& other)
    {
      std::swap_ranges(pdata, pdata + SIZE, other.pdata);
    }

  protected:

    //*************************************************************************
    /// Initialise from an unsigned long long.
    //*************************************************************************
    ibitset& initialise(unsigned long long value)
    {
      reset();

      const size_t SHIFT = (integral_limits<unsigned long long>::bits <= (int)BITS_PER_ELEMENT) ? 0 : BITS_PER_ELEMENT;

      // Can we do it in one hit?
      if (SHIFT == 0)
      {
        pdata[0] = element_t(value);
      }
      else
      {
        size_t i = 0;

        while ((value != 0) && (i < SIZE))
        {
          pdata[i++] = value & ALL_SET;
          value = value >> SHIFT;
        }
      }

      pdata[SIZE - 1] &= TOP_MASK;

      return *this;
    }

    //*************************************************************************
    /// Invert
    //*************************************************************************
    void invert()
    {
      for (size_t i = 0; i < SIZE; ++i)
      {
        pdata[i] = ~pdata[i];
      }
    }

    //*************************************************************************
    /// Gets a reference to the specified bit.
    //*************************************************************************
    bit_reference get_bit_reference(size_t position)
    {
      return bit_reference(*this, position);
    }

    //*************************************************************************
    /// Constructor.
    //*************************************************************************
    ibitset(size_t nbits, size_t size, element_t* pdata)
      : NBITS(nbits),
        SIZE(size),
        pdata(pdata)
    {
      size_t allocated_bits = SIZE * BITS_PER_ELEMENT;
      size_t top_mask_shift = ((BITS_PER_ELEMENT - (allocated_bits - NBITS)) % BITS_PER_ELEMENT);
      TOP_MASK = element_t(top_mask_shift == 0 ? ALL_SET : ~(ALL_SET << top_mask_shift));
    }

    //*************************************************************************
    /// Compare bitsets.
    //*************************************************************************
    static bool is_equal(const ibitset& lhs, const ibitset&rhs)
    {
      return std::equal(lhs.pdata, lhs.pdata + lhs.SIZE, rhs.pdata);
    }

    element_t TOP_MASK;

  private:

    const size_t NBITS;
    const size_t SIZE;
    element_t*   pdata;
  };
}


#if WIN32
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#endif
