/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2016-2024 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2024 TotalEnergies
 * Copyright (c) 2018-2024 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2023-2024 Chevron
 * Copyright (c) 2019-     GEOS/GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file TableFunctionBase.cpp
 */

#include "TableFunctionBase.hpp"
#include "codingUtilities/Parsing.hpp"
#include "common/DataTypes.hpp"

#include <algorithm>

namespace geos
{

TableFunctionBase::TableFunctionBase():
  m_interpolationMethod( InterpolationType::Linear ),
  m_valueUnit( units::Unknown ),
  m_kernelWrapper( createKernelWrapper() )
{}


void TableFunctionBase::setInterpolationType( InterpolationType const method )
{
  m_interpolationMethod = method;
}



void TableFunctionBase::setCoordinates( array1d< real64_array > const & coordinates,
                                        std::vector< units::Unit > const & dimUnits )
{
  m_dimUnits = dimUnits;
  m_coordinates.resize( 0 );
  for( localIndex i = 0; i < coordinates.size(); ++i )
  {
    m_coordinates.appendArray( coordinates[i].begin(), coordinates[i].end() );
  }
}

real64
TableFunctionBase::lowerBound( localIndex const dim ) const
{
  return m_coordinates[dim][0];
}
real64
TableFunctionBase::upperBound( localIndex const dim ) const
{
  return m_coordinates[dim][m_coordinates.sizeOfArray( dim ) - 1];
}
void
TableFunctionBase::setValues( real64_array values, units::Unit unit )
{
  m_values = std::move( values );
  m_valueUnit = unit;
}

std::pair< bool, localIndex >
TableFunctionBase::validateMonotonicIncreasing( )
{

  for( localIndex ii = 0; ii < m_coordinates.size(); ++ii )
  {
    for( localIndex j = 1; j < m_coordinates[ii].size(); ++j )
    {
      if( m_coordinates[ii][j] - m_coordinates[ii][j-1] <= 0 )
      {
        return std::pair< bool, localIndex >( false, ii );
      }
    }
  }
  return std::pair< bool, localIndex >( true, -1 );
}

bool
TableFunctionBase::validateNumberOfEntries( )
{
  localIndex increment = 1;
  for( localIndex ii = 0; ii < m_coordinates.size(); ++ii )
  {
    increment *= m_coordinates.sizeOfArray( ii );
  }
  return increment == m_values.size();
}

bool
TableFunctionBase::validateDimension( localIndex const dim ) const
{
  return dim >= m_coordinates.size() || dim < 0;
}

bool
TableFunctionBase::validateCoordinate( real64 const coord, localIndex const dim ) const
{
  real64 const lowerBound = m_coordinates[dim][0];
  real64 const upperBound = m_coordinates[dim][m_coordinates.sizeOfArray( dim ) - 1];
  return coord > upperBound || coord < lowerBound;
}



TableFunctionBase::KernelWrapper
TableFunctionBase::createKernelWrapper() const
{
  return { m_interpolationMethod,
           m_coordinates.toViewConst(),
           m_values.toViewConst() };
}



TableFunctionBase::KernelWrapper::KernelWrapper( InterpolationType const interpolationMethod,
                                                 ArrayOfArraysView< real64 const > const & coordinates,
                                                 arrayView1d< real64 const > const & values )
  :
  m_interpolationMethod( interpolationMethod ),
  m_coordinates( coordinates ),
  m_values( values )
{}



} // end of namespace geos
