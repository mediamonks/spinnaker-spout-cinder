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

#ifndef FLIR_SPINNAKER_IMAGESTATISTICS_H
#define FLIR_SPINNAKER_IMAGESTATISTICS_H

#include "Interface/IImageStatistics.h"

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

    class SPINNAKER_API ImageStatistics: public IImageStatistics
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
        virtual void EnableAll() override;

        /**
         * Disable all channels.
         *
         */
        virtual void DisableAll() override;

        /**
         * Enable only the grey channel.
         *
         */
        virtual void EnableGreyOnly() override;

        /**
         * Enable only the RGB channels.
         *
         */
        virtual void EnableRGBOnly() override;

        /**
         * Enable only the HSL channels.
         *
         */
        virtual void EnableHSLOnly() override;

        /**
         * Gets the status of a statistics channel.
         *
         * @param channel The statistics channel.
         * @param pEnabled Whether the channel is enabled.
         *
         * @see SetChannelStatus()
         *
         */
        virtual void GetChannelStatus(
            StatisticsChannel channel,
            bool* pEnabled ) const override;

        /**
         * Sets the status of a statistics channel.
         *
         * @param channel The statistics channel.
         * @param enabled Whether the channel should be enabled.
         *
         * @see GetChannelStatus()
         *
         */
        virtual void SetChannelStatus(
            StatisticsChannel channel,
            bool enabled ) override;

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
        virtual void GetRange(
            StatisticsChannel channel,
            unsigned int* pMin,
            unsigned int* pMax ) const override;

        /**
         * Gets the range of a statistics channel. The values returned
         * are the maximum values recorded for all pixels in the image.
         *
         * @param channel The statistics channel.
         * @param pPixelValueMin The minimum pixel value.
         * @param pPixelValueMax The maximum pixel value.
         *
         */
        virtual void GetPixelValueRange(
            StatisticsChannel channel,
            unsigned int* pPixelValueMin,
            unsigned int* pPixelValueMax ) const override;

        /**
         * Gets the number of unique pixel values in the image.
         *
         * @param channel The statistics channel.
         * @param pNumPixelValues The number of unique pixel values.
         *
         */
        virtual void GetNumPixelValues(
            StatisticsChannel channel,
            unsigned int* pNumPixelValues ) const override;

        /**
         * Gets the mean of the image.
         *
         * @param channel The statistics channel.
         * @param pPixelValueMean The mean of the image.
         *
         */
        virtual void GetMean(
            StatisticsChannel channel,
            float* pPixelValueMean ) const override;

        /**
         * Gets the histogram for the image.
         *
         * @param channel The statistics channel.
         * @param ppHistogram Pointer to an array containing the histogram.
         *
         */
        virtual void GetHistogram(
            StatisticsChannel channel,
            int** ppHistogram ) const override;

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
        virtual void GetStatistics(
            StatisticsChannel channel,
            unsigned int* pRangeMin = NULL,
            unsigned int* pRangeMax = NULL,
            unsigned int* pPixelValueMin = NULL,
            unsigned int* pPixelValueMax = NULL,
            unsigned int* pNumPixelValues = NULL,
            float* pPixelValueMean = NULL,
            int** ppHistogram = NULL ) const override;

    private:
        friend class ImageStatsCalculator;
        struct ImageStatisticsData;
        ImageStatisticsData* m_pData;
    };

    /*@}*/

    /*@}*/
}

#endif // FLIR_SPINNAKER_IMAGESTATISTICS_H
