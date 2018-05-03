//=============================================================================
// Copyright © 2017 FLIR Integrated Imaging Solutions, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of FLIR
// Integrated Imaging Solutions, Inc. ("Confidential Information"). You
// shall not disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with FLIR Integrated Imaging Solutions, Inc. (FLIR).
//
// FLIR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. FLIR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

#ifndef PGR_SPINNAKER_IMAGESTATISTICS_H
#define PGR_SPINNAKER_IMAGESTATISTICS_H

#include "Image.h"
//#include <boost/shared_ptr.hpp>

namespace Spinnaker
{
	class ImageStatsCalculator;

	/**
	 * @defgroup SpinnakerClasses Spinnaker Classes
	 */

	/*@{*/

	/**
	 * @defgroup ImageStatistics_h ImageStatistics Class
	 */

	/*@{*/

    /**
     * @brief Represents image statistics for an image.
     */
	
    class  SPINNAKER_API ImageStatistics
    {
    public:

        /**
         * Default constructor.
         */
        ImageStatistics();

        /**
         * Default destructor.
         */
        virtual ~ImageStatistics();

        /**
         * Copy constructor.
         */
        ImageStatistics( const ImageStatistics& other );

        /**
         * Assignment operator.
         *
         * @param other The ImageStatistics object to copy from.
         */
        ImageStatistics& operator=( const ImageStatistics& other );

        /**
         * Enable all channels.
         *
         */
        void EnableAll();

        /**
         * Disable all channels.
         *
         */
        void DisableAll();

        /**
         * Enable only the grey channel.
         *
         */
        void EnableGreyOnly();

        /**
         * Enable only the RGB channels.
         *
         */
        void EnableRGBOnly();

        /**
         * Enable only the HSL channels.
         *
         */
        void EnableHSLOnly();

        /**
         * Gets the status of a statistics channel.
         *
         * @param channel The statistics channel.
         * @param pEnabled Whether the channel is enabled.
         *
         * @see SetChannelStatus()
         *
         */
        void GetChannelStatus(
            StatisticsChannel channel,
            bool* pEnabled ) const;

        /**
         * Sets the status of a statistics channel.
         *
         * @param channel The statistics channel.
         * @param enabled Whether the channel should be enabled.
         *
         * @see GetChannelStatus()
         *
         */
        void SetChannelStatus(
            StatisticsChannel channel,
            bool enabled );

        /**
         * Gets the range of a statistics channel. The values returned
         * are the maximum possible values for any given pixel in the image.
         * This is generally 0-255 for 8 bit images, and 0-65535 for
         * 16 bit images.
         *
         * @param channel The statistics channel.
         * @param pMin The minimum possible value.
         * @param pMax The maximum possible value.
         *
         */
        void GetRange(
            StatisticsChannel channel,
            unsigned int* pMin,
            unsigned int* pMax ) const;

        /**
         * Gets the range of a statistics channel. The values returned
         * are the maximum values recorded for all pixels in the image.
         *
         * @param channel The statistics channel.
         * @param pPixelValueMin The minimum pixel value.
         * @param pPixelValueMax The maximum pixel value.
         *
         */
        void GetPixelValueRange(
            StatisticsChannel channel,
            unsigned int* pPixelValueMin,
            unsigned int* pPixelValueMax ) const;

        /**
         * Gets the number of unique pixel values in the image.
         *
         * @param channel The statistics channel.
         * @param pNumPixelValues The number of unique pixel values.
         *
         */
        void GetNumPixelValues(
            StatisticsChannel channel,
            unsigned int* pNumPixelValues ) const;

        /**
         * Gets the mean of the image.
         *
         * @param channel The statistics channel.
         * @param pPixelValueMean The mean of the image.
         *
         */
        void GetMean(
            StatisticsChannel channel,
            float* pPixelValueMean ) const;

        /**
         * Gets the histogram for the image.
         *
         * @param channel The statistics channel.
         * @param ppHistogram Pointer to an array containing the histogram.
         *
         */
        void GetHistogram(
            StatisticsChannel channel,
            int** ppHistogram ) const;

        /**
         * Gets all statistics for the image.
         *
         * @param channel The statistics channel.
         * @param pRangeMin The minimum possible value.
         * @param pRangeMax The maximum possible value.
         * @param pPixelValueMin The minimum pixel value.
         * @param pPixelValueMax The maximum pixel value.
         * @param pNumPixelValues The number of unique pixel values.
         * @param pPixelValueMean The mean of the image.
         * @param ppHistogram Pointer to an array containing the histogram.
         *
         */
        void GetStatistics(
            StatisticsChannel channel,
            unsigned int* pRangeMin = NULL,
            unsigned int* pRangeMax = NULL,
            unsigned int* pPixelValueMin = NULL,
            unsigned int* pPixelValueMax = NULL,
            unsigned int* pNumPixelValues = NULL,
            float* pPixelValueMean = NULL,
            int** ppHistogram = NULL ) const;
    private:
        friend class ImageStatsCalculator;
        struct ImageStatisticsData;
        ImageStatisticsData* m_pData;
    };

	/*@}*/

	/*@}*/
}

#endif // PGR_SPINNAKER_IMAGESTATISTICS_H
