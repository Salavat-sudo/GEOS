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
 * @file TableFunction.hpp
 */

#ifndef GEOS_FUNCTIONS_TABLEFUNCTION_HPP_
#define GEOS_FUNCTIONS_TABLEFUNCTION_HPP_

#include "FunctionBase.hpp"
#include "TableFunctionBase.hpp"
#include "common/format/EnumStrings.hpp"
#include "LvArray/src/tensorOps.hpp"
#include "common/format/table/TableFormatter.hpp"
#include "common/Units.hpp"

namespace geos
{

/**
 * @class TableFunction
 *
 * An interface for a dense table-based function
 */
class TableFunction : public TableFunctionBase, public FunctionBase
{
public:

  /// Struct containing output options
  struct OutputOptions
  {
    /// Output PVT in CSV file
    bool writeCSV;
    /// Output PVT in log
    bool writeInLog;
  };


  /**
   * @brief The constructor
   * @param[in] name the name of this object manager
   * @param[in] parent the parent Group
   */
  TableFunction( const string & name,
                 dataRepository::Group * const parent );

/**
 * @brief The catalog name interface
 * @return name of the TableFunction in the FunctionBase catalog
 */
  static string catalogName() { return "TableFunction"; }

  /**
   * @brief Initialize the table function
   */
  virtual void initializeFunction() override;

  /**
   * @brief Build the maps used to evaluate the table function
   */
  void reInitializeFunction();

  /**
   * @brief Set the interpolation method
   * @param method The interpolation method
   */
  void setInterpolationMethod( InterpolationType const method );

  /**
   * @brief Set the table coordinates
   * @param coordinates An array of arrays containing table coordinate definitions
   * @param dimUnits The units of each dimension of the coordinates, in the same order
   */
  void setTableCoordinates( array1d< real64_array > const & coordinates,
                            std::vector< units::Unit > const & dimUnits = {} );

  /**
   * @brief Set the table values
   * @param values An array of table values in fortran order
   * @param unit The unit of the given values
   */

  void setTableValues( real64_array values, units::Unit unit = units::Unknown );


  /**
   * @brief Check if the given coordinate is in the bounds of the table coordinates in the
   * specified dimension, throw an exception otherwise.
   * @param coord the coordinate in the 'dim' dimension that must be checked
   * @param dim the dimension in which the coordinate must be checked
   * @throw SimulationError if the value is out of the coordinates bounds.
   */
  void checkCoord( real64 coord, localIndex dim ) const;


  /**
   * @brief Method to evaluate a function
   * @param input a scalar input
   * @return the function result
   */
  virtual real64 evaluate( real64 const * const input ) const override final;


  /**
   * @brief Print the table(s) in the log and/or CSV files when requested by the user.
   * @param pvtOutputOpts Struct containing output options
   */
  void outputPVTTableData( OutputOptions const pvtOutputOpts ) const;

private:

  friend class FunctionBase;



  /**
   * @brief Method to evaluate a function on a target object
   * @param group a pointer to the object holding the function arguments
   * @param time current time
   * @param set the subset of nodes to apply the function to
   * @param result an array to hold the results of the function
   */
  virtual void evaluate( dataRepository::Group const & group,
                         real64 const time,
                         SortedArrayView< localIndex const > const & set,
                         arrayView1d< real64 > const & result ) const override final
  {
    FunctionBase::evaluateT< TableFunction, parallelHostPolicy >( group, time, set, result );
  }



  /// Struct containing lookup keys for data repository wrappers
  struct viewKeyStruct
  {
    /// @return Key for coordinate arrays
    static constexpr char const * coordinatesString() { return "coordinates"; }
    /// @return Key for value array
    static constexpr char const * valuesString() { return "values"; }
    /// @return Key for interpolation type
    static constexpr char const * interpolationString() { return "interpolation"; }
    /// @return Key for list of files containing table coordinates
    static constexpr char const * coordinateFilesString() { return "coordinateFiles"; }
    /// @return Key for name of file containing table values
    static constexpr char const * voxelFileString() { return "voxelFile"; }
  };

private:

  /**
   * @brief Parse a table file.
   * @param[in] target The place to store values.
   * @param[in] filename The name of the file to read.
   * @param[in] delimiter The delimiter used for file entries.
   */
  void readFile( string const & filename, array1d< real64 > & target );

  /// Coordinates for 1D table
  array1d< real64 > m_tableCoordinates1D;

  /// List of table coordinate file names
  path_array m_coordinateFiles;

  /// Table voxel file names
  Path m_voxelFile;


};

/**
 * @brief Template specialisation to convert a TableFunction to a CSV string.
 * @param tableData The TableFunction object to convert.
 * @return The CSV string representation of the TableFunction.
 */
template<>
string TableTextFormatter::toString< TableFunction >( TableFunction const & tableData ) const;

/**
 * @brief Template specialisation to convert a TableFunction to a table string.
 * @param tableData The TableFunction object to convert.
 * @return The table string representation of the TableFunction.
 */
template<>
string TableCSVFormatter::toString< TableFunction >( TableFunction const & tableData ) const;

} /* namespace geos */

#endif /* GEOS_FUNCTIONS_TABLEFUNCTION_HPP_ */
