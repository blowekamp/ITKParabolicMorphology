#ifndef __itkBinaryOpenParabolicImageFilter_txx
#define __itkBinaryOpenParabolicImageFilter_txx

#include "itkProgressAccumulator.h"
#include "itkBinaryOpenParaImageFilter.h"
#include "itkParabolicErodeImageFilter.h"
#include "itkProgressAccumulator.h"
#include "itkCropImageFilter.h"
#include "itkConstantPadImageFilter.h"


namespace itk
{

template <typename TInputImage, typename TOutputImage>
BinaryOpenParaImageFilter<TInputImage, TOutputImage>
::BinaryOpenParaImageFilter()
{
  this->SetNumberOfRequiredOutputs( 1 );
  this->SetNumberOfRequiredInputs( 1 );
  this->m_CircErode = CircErodeType::New();
  this->m_CircDilate = CircDilateType::New();
  this->m_CircCastA = CCastTypeA::New();
  this->m_CircCastB = CCastTypeB::New();

  this->m_RectErode = RectErodeType::New();
  this->m_RectDilate = RectDilateType::New();
  this->m_RectCastA = RCastTypeA::New();
  this->m_RectCastB = RCastTypeB::New();
  this->m_Circular = true;
    // Need to call this after filters are created
  this->SetUseImageSpacing(false);
  this->SetSafeBorder(true);

}


template <typename TInputImage, typename TOutputImage>
void
BinaryOpenParaImageFilter<TInputImage, TOutputImage>
::SetRadius( ScalarRealType radius )
{
  RadiusType s;
  s.Fill(radius);
  this->SetRadius( s );
}

template <typename TInputImage, typename TOutputImage >
void
BinaryOpenParaImageFilter<TInputImage, TOutputImage >
::GenerateData(void)
{
  // Allocate the output
  this->AllocateOutputs();
  typename TInputImage::SizeType Pad;

  // set up the scaling before we pass control over to superclass
  if (this->m_RectErode->GetUseImageSpacing())
    {
    // radius is in mm
    RadiusType R;
    for (unsigned P=0;P<InputImageType::ImageDimension;P++)
      {
      R[P] = 0.5*m_Radius[P] * m_Radius[P];
      Pad[P] = round(m_Radius[P]/this->GetInput()->GetSpacing()[P] + 1);
      }
    m_RectErode->SetScale(R);
    m_CircErode->SetScale(R);
    m_RectDilate->SetScale(R);
    m_CircDilate->SetScale(R);

    }
  else
    {
    // radius is in pixels
    RadiusType R;
    // this gives us a little bit of a margin 
    for (unsigned P=0;P<InputImageType::ImageDimension;P++)
      {
      R[P] = (0.5*m_Radius[P] * m_Radius[P]+1);
      Pad[P] = m_Radius[P]+1;
      }
    //std::cout << "no image spacing " << m_Radius << R << std::endl;
    m_RectErode->SetScale(R);
    m_CircErode->SetScale(R);
    m_RectDilate->SetScale(R);
    m_CircDilate->SetScale(R);

    }


  if (m_Circular)
    {
    ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
    progress->SetMiniPipelineFilter(this);
    InputImageConstPointer inputImage;
    inputImage = this->GetInput();

    progress->RegisterInternalFilter(m_CircErode, 0.4f);
    progress->RegisterInternalFilter(m_CircCastA, 0.1f);
    progress->RegisterInternalFilter(m_CircDilate, 0.4f);
    progress->RegisterInternalFilter(m_CircCastB, 0.1f);

    m_CircCastA->SetInput(m_CircErode->GetOutput());
    m_CircCastA->SetVal(1.0);
    
    m_CircDilate->SetInput(m_CircCastA->GetOutput());

    m_CircCastB->SetInput(m_CircDilate->GetOutput());
    m_CircCastB->SetUpperThreshold(0);
    m_CircCastB->SetOutsideValue(1);
    m_CircCastB->SetInsideValue(0);
 
    if (m_SafeBorder)
      {
      typedef typename itk::ConstantPadImageFilter<InputImageType, InputImageType> PadType;
      typename PadType::Pointer pad = PadType::New();
      pad->SetPadLowerBound( Pad);
      pad->SetPadUpperBound( Pad );
      pad->SetConstant( 1 );
      pad->SetInput( inputImage );
      m_CircErode->SetInput(pad->GetOutput());
      typename InputImageType::Pointer pin = pad->GetOutput();
      std::cout << "para erode in" << pin ;
      typedef typename itk::CropImageFilter<TOutputImage, TOutputImage> CropType;
      typename CropType::Pointer crop = CropType::New();
      crop->SetInput( m_CircCastB->GetOutput() );
      crop->SetUpperBoundaryCropSize( Pad );
      crop->SetLowerBoundaryCropSize( Pad );

      crop->GraftOutput( this->GetOutput() );
      crop->Update();
      this->GraftOutput( crop->GetOutput() );

      }
    else
      {
      m_CircErode->SetInput(inputImage);
      
      m_CircCastB->GraftOutput(this->GetOutput());
      m_CircCastB->Update();
      this->GraftOutput(m_CircCastB->GetOutput());
      }
    }
  else
    {
    ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
    progress->SetMiniPipelineFilter(this);
    InputImageConstPointer inputImage;
    inputImage = this->GetInput();

    progress->RegisterInternalFilter(m_RectErode, 0.4f);
    progress->RegisterInternalFilter(m_RectCastA, 0.1f);
    progress->RegisterInternalFilter(m_RectDilate, 0.4f);
    progress->RegisterInternalFilter(m_RectCastB, 0.1f);

    m_RectCastA->SetInput(m_RectErode->GetOutput());
    m_RectCastA->SetVal(1);
    
    m_RectDilate->SetInput(m_RectCastA->GetOutput());
    
    m_RectCastB->SetInput(m_RectDilate->GetOutput());
    m_RectCastB->SetUpperThreshold(0);
    m_RectCastB->SetOutsideValue(1);
    m_RectCastB->SetInsideValue(0);

    if (m_SafeBorder)
      {
      typedef typename itk::ConstantPadImageFilter<InputImageType, InputImageType> PadType;
      typename PadType::Pointer pad = PadType::New();
      pad->SetPadLowerBound( Pad);
      pad->SetPadUpperBound( Pad );
      pad->SetConstant( 1 );
      pad->SetInput( inputImage );
      m_RectErode->SetInput(pad->GetOutput());

      typedef typename itk::CropImageFilter<TOutputImage, TOutputImage> CropType;
      typename CropType::Pointer crop = CropType::New();
      crop->SetInput( m_RectCastB->GetOutput() );
      crop->SetUpperBoundaryCropSize( Pad );
      crop->SetLowerBoundaryCropSize( Pad );

      crop->GraftOutput( this->GetOutput() );
      crop->Update();
      this->GraftOutput( crop->GetOutput() );

      }
    else
      {
      m_RectErode->SetInput(inputImage);
      m_RectCastB->GraftOutput(this->GetOutput());
      m_RectCastB->Update();
      
      this->GraftOutput(m_RectCastB->GetOutput());
      }

    }
}


template <typename TInputImage, typename TOutputImage>
void
BinaryOpenParaImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  if (this->m_Circular)
    {
    os << "Circular opening, ";
    }
  else
    {
    os << "Rectangular opening, ";
    }

  if (this->m_SafeBorder)
    {

    os << "safe border" << std::endl;
    }
  else
    {
    os << "unsafe border" << std::endl;
    }

  if (this->m_CircErode->GetUseImageSpacing())
    {
    os << "Radius in world units: " << this->GetRadius() << std::endl;
    }
  else
    {
    os << "Radius in voxels: " << this->GetRadius() << std::endl;
    }
}


} // namespace itk
#endif