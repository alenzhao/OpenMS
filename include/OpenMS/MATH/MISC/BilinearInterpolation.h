// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// --------------------------------------------------------------------------
//                   OpenMS Mass Spectrometry Framework
// --------------------------------------------------------------------------
//  Copyright (C) 2003-2007 -- Oliver Kohlbacher, Knut Reinert
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// --------------------------------------------------------------------------
// $Maintainer: Clemens Groepl $
// --------------------------------------------------------------------------

#ifndef OPENMS_MATH_MISC_BILINEARINTERPOLATION_H
#define OPENMS_MATH_MISC_BILINEARINTERPOLATION_H

#include <OpenMS/DATASTRUCTURES/Matrix.h>

namespace OpenMS
{

	namespace Math
	{

		/**
			 @brief Provides access to bilinearly interpolated values (and
			 derivatives) from discrete data points.  Values beyond the given range
			 of data points are implicitly taken as zero.

			 The input is just a vector of values ("Data").  These are interpreted
			 as the y-coordinates at the x-coordinate positions 0,...,data_.size-1.

			 The interpolated data can also be <i>scaled</i> and <i>shifted</i> in
			 the x-dimension by an <em>affine mapping</em>.  That is, we have "inside" and
			 "outside" x-coordinates.  The affine mapping can be specified in two
			 ways:
			 - using setScale() and setOffset(),
			 - using setMapping()
			 .
			 By default the identity mapping (scale=1, offset=0) is used.

			 Using the value() and derivative() methods you can sample bilinearly
			 interpolated values for a given x-coordinate position of the data and
			 the derivative of the data.

			 @see LinearInterpolation


			 @ingroup Math
		*/
		template < typename Key = double, typename Value = Key >
		class BilinearInterpolation
		{

		 public:

			///\name Typedefs
			//@{
			typedef Value value_type;

			typedef Key key_type;
			typedef Matrix < value_type > container_type;

			typedef value_type      ValueType;
			typedef key_type        KeyType;
			typedef container_type  ContainerType;
			//@}

		 public:

			/**@brief Constructors and destructor.

			The first argument is the scale which is applied to the arguments of
			value() and derivative() before looking up the interpolated values in
			the container.  The second argument is the offset, which is
			subtracted before everything else.
			*/
			BilinearInterpolation ( KeyType scale_0 = 1., KeyType offset_0 = 0.,
															KeyType scale_1 = 1., KeyType offset_1 = 0.
														)
				: scale_0_  ( scale_0 ),
					offset_0_ ( offset_0 ),
					scale_1_  ( scale_1 ),
					offset_1_ ( offset_1 ),
					inside_0_  ( 0 ),
					outside_0_ ( 0 ),
					inside_1_  ( 1 ),
					outside_1_ ( 1 ),
					data_   ()
			{}

			/// Copy constructor.
			BilinearInterpolation ( BilinearInterpolation const & arg )
				: scale_0_  ( arg.scale_0_ ),
					offset_0_ ( arg.offset_0_ ),
					scale_1_  ( arg.scale_1_ ),
					offset_1_ ( arg.offset_1_ ),
					inside_0_ ( arg.inside_0_ ),
					outside_0_ (arg.outside_0_ ),
					inside_1_ ( arg.inside_1_ ),
					outside_1_ (arg.outside_1_ ),
					data_   ( arg.data_ )
			{}

			/// Assignment operator.
			BilinearInterpolation & operator= ( BilinearInterpolation const & arg )
			{
				if ( &arg == this ) return *this;
				scale_0_   = arg.scale_0_;
				offset_0_  = arg.offset_0_;
				scale_1_   = arg.scale_1_;
				offset_1_  = arg.offset_1_;
				inside_0_  = arg.inside_0_;
				outside_1_ = arg.outside_1_;
				inside_1_  = arg.inside_1_;
				outside_0_ = arg.outside_0_;
				data_      = arg.data_;
				return *this;
			}

			/// Destructor.
			~BilinearInterpolation () {}

			// ----------------------------------------------------------------------

			///@name Interpolated data
			//@{

			/// Returns the interpolated value.
			ValueType value ( KeyType arg_pos_0, KeyType arg_pos_1 ) const throw()
			{
				// apply the key transformations
				KeyType const pos_0 = key2index_0(arg_pos_0);
				KeyType const pos_1 = key2index_1(arg_pos_1);


				// ???? should use modf() here!

				int const size_0 = data_.rows();
				int const lower_0 = int(pos_0); // this rounds towards zero
				int const size_1 = data_.cols();
				int const lower_1 = int(pos_1); // this rounds towards zero

				// small pos_0
				if ( pos_0 <= 0 )
				{
					if ( lower_0 != 0 )
					{
						return 0;
					}
					else
					{ // that is: -1 < pos_0 <= 0

						// small pos_1
						if ( pos_1 <= 0 )
						{
							if ( lower_1 != 0 )
							{
								return 0;
							}
							else
							{ // that is: -1 < pos_1 <= 0
								return data_( 0, 0 ) * ( 1. + pos_0 ) * ( 1. + pos_1 ) ;
							}
						}

						// big pos_1
						if ( lower_1 >= size_1 - 1 )
						{
							if ( lower_1 != size_1 - 1 )
							{
								return 0;
							}
							else
							{
								return data_ ( 0, lower_1 ) * ( 1. + pos_0 ) * ( size_1 - pos_1 );
							}
						}

						// mediumm pos_1
						KeyType const factor_1 = pos_1 - KeyType(lower_1);
						KeyType const factor_1_complement = 1. - factor_1;
						return
							(
							 data_ ( 0, lower_1 + 1 ) * factor_1 +
							 data_ ( 0, lower_1 ) * factor_1_complement
							) * ( 1. + pos_0 );
					}
				}

				// big pos_0
				if ( lower_0 >= size_0 - 1 )
				{
					if ( lower_0 != size_0 - 1 )
					{
						return 0;
					}
					else
					{ // that is: size_0 - 1 <= pos_0 < size_0

						// small pos_1
						if ( pos_1 <= 0 )
						{
							if ( lower_1 != 0 )
							{
								return 0;
							}
							else
							{ // that is: -1 < pos_1 <= 0
								return data_( lower_0, 0 ) * ( size_0 - pos_0 ) * ( 1. + pos_1 ) ;
							}
						}

						// big pos_1
						if ( lower_1 >= size_1 - 1 )
						{
							if ( lower_1 != size_1 - 1 )
							{
								return 0;
							}
							else
							{
								return data_ ( lower_0, lower_1 ) * ( size_0 - pos_0 ) * ( size_1 - pos_1 );
							}
						}

						// mediumm pos_1
						KeyType const factor_1 = pos_1 - KeyType(lower_1);
						KeyType const factor_1_complement = 1. - factor_1;
						return
							(
							 data_ ( lower_0, lower_1 + 1 ) * factor_1 +
							 data_ ( lower_0, lower_1 ) * factor_1_complement
							)
							* ( size_0 - pos_0 );
					}
				}

				// medium pos_0
				{
					KeyType const factor_0 = pos_0 - KeyType(lower_0);
					KeyType const factor_0_complement = 1. - factor_0;

					// small pos_1
					if ( pos_1 <= 0 )
					{
						if ( lower_1 != 0 )
						{
							return 0;
						}
						else
						{ // that is: -1 < pos_1 <= 0
							return
								(
								 data_( lower_0 + 1, 0 ) * factor_0
								 +
								 data_( lower_0, 0 ) * factor_0_complement
								)
								* ( 1. + pos_1 ) ;
						}
					}

					// big pos_1
					if ( lower_1 >= size_1 - 1 )
					{
						if ( lower_1 != size_1 - 1 )
						{
							return 0;
						}
						else
						{
							return
								(
								 data_( lower_0 + 1, lower_1 ) * factor_0
								 +
								 data_( lower_0, lower_1 ) * factor_0_complement
								)
								* ( size_1 - pos_1 );
						}
					}
					KeyType const factor_1 = pos_1 - KeyType(lower_1);
					KeyType const factor_1_complement = 1. - factor_1;

					// medium pos_0 and medium pos_1 --> "within" the matrix
					return
						(
						 data_( lower_0 + 1, lower_1 + 1 ) * factor_0
						 +
						 data_( lower_0, lower_1 + 1 ) * factor_0_complement
						)
						* factor_1
						+
						(
						 data_( lower_0 + 1, lower_1 ) * factor_0
						 +
						 data_( lower_0, lower_1 ) * factor_0_complement
						)
						* factor_1_complement;
				}
			}

			/**@brief Performs bilinear resampling.  The arg_value is split up and
				 added to the data points around arg_pos.
			*/
			void addValue ( KeyType arg_pos_0, KeyType arg_pos_1, ValueType arg_value ) throw()
			{

				typedef typename container_type::difference_type DiffType;

				// apply key transformation _0
				KeyType const pos_0 = key2index_0(arg_pos_0);
				KeyType lower_0_key;
				KeyType const frac_0 = std::modf(pos_0, &lower_0_key);
				DiffType const lower_0 = DiffType(lower_0_key);

				// Small pos_0 ?
				if ( pos_0 < 0 )
				{
					if ( lower_0 )
					{
						return;
					}
					else
					{ // lower_0 == 0

						// apply key transformation _1
						KeyType const pos_1 = key2index_1(arg_pos_1);
						KeyType lower_1_key;
						KeyType const frac_1 = std::modf(pos_1, &lower_1_key);
						DiffType const lower_1 = DiffType(lower_1_key);

						// Small pos_1 ?
						if ( pos_1 < 0 )
						{
							if ( lower_1 )
							{
								return;
							}
							else
							{ // lower_1 == 0
								data_( 0, 0 ) += arg_value * ( 1 + frac_0 ) * ( 1 + frac_1 );
								return;
							}
						}
						else // pos_1 >= 0
						{
							DiffType const back_1 = data_.cols() - 1;
							// big pos_1
							if ( lower_1 >= back_1 )
							{
								if ( lower_1 != back_1 )
								{
									return;
								}
								else // lower_1 == back_1
								{
									data_ ( 0, lower_1 ) += arg_value * ( 1 + frac_0 ) * ( 1 - frac_1 );
									return;
								}
							}
							else
							{
								// medium pos_1
								KeyType const tmp_prod = arg_value * ( 1 + frac_0 );
								data_ ( 0, lower_1 + 1 ) += tmp_prod * frac_1;
								data_ ( 0, lower_1 ) += tmp_prod * ( 1 - frac_1 );
								return;
							}
						}
					}
				}
				else // pos_0 >= 0
				{
					DiffType const back_0 = data_.rows() - 1;
					if ( lower_0 >= back_0 )
					{
						if ( lower_0 != back_0 )
						{
							return;
						}
						else // lower_0 == back_0
						{

							KeyType const tmp_prod = arg_value * ( 1 - frac_0 );

							// apply key transformation _1
							KeyType const pos_1 = key2index_1(arg_pos_1);
							KeyType lower_1_key;
							KeyType const frac_1 = std::modf(pos_1, &lower_1_key);
							DiffType const lower_1 = DiffType(lower_1_key);

							// Small pos_1 ?
							if ( pos_1 < 0 )
							{
								if ( lower_1 )
								{
									return;
								}
								else
								{ // lower_1 == 0
									data_( lower_0, 0 ) += tmp_prod * ( 1 + frac_1 );
									return;
								}
							}
							else // pos_1 >= 0
							{
								DiffType const back_1 = data_.cols() - 1;
								// big pos_1
								if ( lower_1 >= back_1 )
								{
									if ( lower_1 != back_1 )
									{
										return;
									}
									else // lower_1 == back_1
									{
										data_ ( lower_0, lower_1 ) += tmp_prod * ( 1 - frac_1 );
										return;
									}
								}
								else
								{
									// medium pos_1
									data_ ( lower_0, lower_1 + 1 ) += tmp_prod * frac_1;
									data_ ( lower_0, lower_1 ) += tmp_prod * ( 1 - frac_1 );
									return;
								}
							}
						}
					}
					else // lower_0 < back_0
					{

						// Medium pos_0 !

						// apply key transformation _1
						KeyType const pos_1 = key2index_1(arg_pos_1);
						KeyType lower_1_key;
						KeyType const frac_1 = std::modf(pos_1, &lower_1_key);
						DiffType const lower_1 = DiffType(lower_1_key);

						// Small pos_1 ?
						if ( pos_1 < 0 )
						{
							if ( lower_1 )
							{
								return;
							}
							else
							{ // lower_1 == 0
								KeyType const tmp_prod = arg_value * ( 1 + frac_1 );
								data_( lower_0 + 1, 0 ) += tmp_prod * frac_0;
								data_( lower_0, 0 ) += tmp_prod * ( 1 - frac_0 );
								return;
							}
						}
						else // pos_1 >= 0
						{
							DiffType const back_1 = data_.cols() - 1;
							// big pos_1
							if ( lower_1 >= back_1 )
							{
								if ( lower_1 != back_1 )
								{
									return;
								}
								else // lower_1 == back_1
								{
									KeyType const tmp_prod = arg_value * ( 1 - frac_1 );
									data_ ( lower_0 + 1, lower_1 ) += tmp_prod * frac_0;
									data_ ( lower_0, lower_1 ) += tmp_prod * ( 1 - frac_0 );
									return;
								}
							}
							else
							{
								// Medium pos_1 !

								// medium pos_0 and medium pos_1 --> "within" the matrix
								KeyType tmp_prod = arg_value * frac_0;
								data_( lower_0 + 1, lower_1 + 1 ) += tmp_prod * frac_1;
								data_( lower_0 + 1, lower_1 ) += tmp_prod * ( 1 - frac_1 );
								tmp_prod = arg_value * ( 1 - frac_0 );
								data_( lower_0, lower_1 + 1 ) += tmp_prod * frac_1;
								data_( lower_0, lower_1 ) += tmp_prod * ( 1 - frac_1 );
								return;
							}
						}
					}
				}
			}

			//@}

			// ----------------------------------------------------------------------

			///@name Discrete (non-interpolated) data
			//@{

			/// Returns the internal random access container from which interpolated values are being sampled.
			ContainerType & getData () throw()
			{
				return data_;
			}

			/// Returns the internal random access container from which interpolated values are being sampled.
			ContainerType const & getData () const throw()
			{
				return data_;
			}

			/**@brief Assigns data to the internal random access container from
				 which interpolated values are being sampled.

				 SourceContainer must be assignable to ContainerType.
			*/
			template < typename SourceContainer >
			void setData ( SourceContainer const & data ) throw()
			{
				data_ = data;
			}


			/// Returns \c true if getData() is empty.
			bool empty () const throw() { return data_.empty(); }

			//@}

			// ----------------------------------------------------------------------

			///\name Transformation
			//@{

			/// The transformation from "outside" to "inside" coordinates.
			KeyType key2index_0 ( KeyType pos ) const throw()
			{
				if ( scale_0_ )
				{
					pos -= offset_0_;
					pos /= scale_0_;
					return pos;
				}
				else
				{
					return 0;
				}
			}

			/// The transformation from "inside" to "outside" coordinates.
			KeyType index2key_0 ( KeyType pos ) const throw()
			{
				pos *= scale_0_;
				pos += offset_0_;
				return pos;
			}

			/// The transformation from "outside" to "inside" coordinates.
			KeyType key2index_1 ( KeyType pos ) const throw()
			{
				if ( scale_1_ )
				{
					pos -= offset_1_;
					pos /= scale_1_;
					return pos;
				}
				else
				{
					return 0;
				}
			}

			/// The transformation from "inside" to "outside" coordinates.
			KeyType index2key_1 ( KeyType pos ) const throw()
			{
				pos *= scale_1_;
				pos += offset_1_;
				return pos;
			}

			/// Accessor.  "Scale" is the difference (in "outside" units) between consecutive entries in "Data".
			KeyType const & getScale_0 () const throw(){ return scale_0_; }

			/// Accessor.  "Scale" is the difference (in "outside" units) between consecutive entries in "Data".
			KeyType const & getScale_1 () const throw(){ return scale_1_; }

			/**@brief Accessor.  "Scale" is the difference (in "outside" units) between consecutive entries in "Data".

			<b>Note:</b> Using this invalidates the inside and outside reference
			points.
			*/
			void setScale_0 ( KeyType const & scale ) throw() { scale_0_ = scale; }

			/**@brief Accessor.  "Scale" is the difference (in "outside" units) between consecutive entries in "Data".

			<b>Note:</b> Using this invalidates the inside and outside reference
			points.
			*/
			void setScale_1 ( KeyType const & scale ) throw() { scale_1_ = scale; }

			/// Accessor.  "Offset" is the point (in "outside" units) which corresponds to "Data(0,0)".
			KeyType const & getOffset_0 () const throw() { return offset_0_; }

			/// Accessor.  "Offset" is the point (in "outside" units) which corresponds to "Data(0,0)".
			KeyType const & getOffset_1 () const throw() { return offset_1_; }

			/**@brief Accessor.  "Offset" is the point (in "outside" units) which
				 corresponds to "Data(0,0)".

				 <b>Note:</b> Using this invalidates the inside and outside reference
				 points.
			*/
			void setOffset_0 ( KeyType const & offset ) throw() { offset_0_ = offset; }

			/**@brief Accessor.  "Offset" is the point (in "outside" units) which
				 corresponds to "Data(0,0)".

				 <b>Note:</b> Using this invalidates the inside and outside reference
				 points.
			*/
			void setOffset_1 ( KeyType const & offset ) throw() { offset_1_ = offset; }

			/**@brief Specifies the mapping from "outside" to "inside" coordinates by the following data:
				 - <code>scale</code>: the difference in outside coordinates between consecutive values in the data vector.
				 - <code>inside</code> and <code>outside</code>: these axis positions are mapped onto each other.

				 For example, when you have a complicated probability distribution
				 which is in fact centered around zero (but you cannot have negative
				 indices in the data vector), then you can arrange things such that
				 inside is the mean of the pre-computed, shifted density values of that
				 distribution and outside is the centroid position of, say, a peak in
				 the real world which you want to model by a scaled and shifted version
				 of the probability distribution.

			*/
			void setMapping_0 ( KeyType const & scale, KeyType const & inside, KeyType const & outside )
			{
				scale_0_   = scale;
				inside_0_  = inside;
				outside_0_ = outside;
				offset_0_  = outside - scale * inside;
				return;
			}

			/**@brief Specifies the mapping from "outside" to "inside" coordinates by the following data:
				 - <code>inside_low</code> and <code>outside_low</code>: these axis positions are mapped onto each other.
				 - <code>inside_high</code> and <code>outside_high</code>: these axis positions are mapped onto each other.

				 This four argument version is just a convenience overload for the three argument version, which see.
			*/
			void setMapping_0 ( KeyType const & inside_low, KeyType const & outside_low,
													KeyType const & inside_high, KeyType const & outside_high )
			{
				if ( inside_high != inside_low )
				{
					setMapping_0 ( ( outside_high - outside_low ) / ( inside_high - inside_low ),
												 inside_low, outside_low );
				}
				else
				{
					setMapping_0 ( 0, inside_low, outside_low );
				}
				return;
			}

			/**@brief Specifies the mapping from "outside" to "inside" coordinates by the following data:
				 - <code>scale</code>: the difference in outside coordinates between consecutive values in the data vector.
				 - <code>inside</code> and <code>outside</code>: these axis positions are mapped onto each other.

				 For example, when you have a complicated probability distribution
				 which is in fact centered around zero (but you cannot have negative
				 indices in the data vector), then you can arrange things such that
				 inside is the mean of the pre-computed, shifted density values of that
				 distribution and outside is the centroid position of, say, a peak in
				 the real world which you want to model by a scaled and shifted version
				 of the probability distribution.

			*/
			void setMapping_1 ( KeyType const & scale, KeyType const & inside, KeyType const & outside )
			{
				scale_1_   = scale;
				inside_1_  = inside;
				outside_1_ = outside;
				offset_1_  = outside - scale * inside;
			}

			/**@brief Specifies the mapping from "outside" to "inside" coordinates by the following data:
				 - <code>inside_low</code> and <code>outside_low</code>: these axis positions are mapped onto each other.
				 - <code>inside_high</code> and <code>outside_high</code>: these axis positions are mapped onto each other.

				 This four argument version is just a convenience overload for the three argument version, which see.
			*/
			void setMapping_1 ( KeyType const & inside_low, KeyType const & outside_low,
													KeyType const & inside_high, KeyType const & outside_high )
			{
				if ( inside_high != inside_low )
				{
					setMapping_1 ( ( outside_high - outside_low ) / ( inside_high - inside_low ),
												 inside_low, outside_low );
				}
				else
				{
					setMapping_1 ( 0, inside_low, outside_low );
				}
				return;
			}

			/// Accessor.  See setMapping().
			KeyType const & getInsideReferencePoint_0 () const throw() { return inside_0_; }

			/// Accessor.  See setMapping().
			KeyType const & getInsideReferencePoint_1 () const throw() { return inside_1_; }

			/// Accessor.  See setMapping().
			KeyType const & getOutsideReferencePoint_0 () const throw() { return outside_0_; }

			/// Accessor.  See setMapping().
			KeyType const & getOutsideReferencePoint_1 () const throw() { return outside_1_; }

			/// Lower boundary of the support, in "outside" coordinates.
			KeyType supportMin_0() const throw()
			{ return index2key_0 ( empty() ? 0 : -1 ); }

			/// Lower boundary of the support, in "outside" coordinates.
			KeyType supportMin_1() const throw()
			{ return index2key_1 ( empty() ? 0 : -1 ); }

			/// Upper boundary of the support, in "outside" coordinates.
			KeyType supportMax_0() const throw()
			{ return index2key_0 ( data_.cols() ); }

			/// Upper boundary of the support, in "outside" coordinates.
			KeyType supportMax_1() const throw()
			{ return index2key_1 ( data_.rows() ); }

			//@}

		 protected:

			KeyType scale_0_;
			KeyType offset_0_;
			KeyType scale_1_;
			KeyType offset_1_;
			KeyType inside_0_;
			KeyType outside_0_;
			KeyType inside_1_;
			KeyType outside_1_;

			ContainerType data_;

		};

	} // namespace Math

} // namespace OpenMS

#endif // OPENMS_MATH_MISC_BILINEARINTERPOLATION_H
