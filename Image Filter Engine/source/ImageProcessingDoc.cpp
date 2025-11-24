#include "stdafx.h"

#undef min
#undef max

#include "ImageProcessing.h"
#include "ImageProcessingDoc.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CImageProcessingDoc, CDocument)

BEGIN_MESSAGE_MAP(CImageProcessingDoc, CDocument)

	ON_COMMAND(ID_FILE_REVERT, &CImageProcessingDoc::OnFileRevert)

	ON_COMMAND(ID_STYLIZE_CARTOON, &CImageProcessingDoc::OnStylizeCartoon)
	ON_COMMAND(ID_STYLIZE_OILPAINTING, &CImageProcessingDoc::OnStylizeOilPainting)
	ON_COMMAND(ID_STYLIZE_POPART, &CImageProcessingDoc::OnStylizePopArt)
	ON_COMMAND(ID_STYLIZE_EMBOSS, &CImageProcessingDoc::OnStylizeEmboss)

	ON_COMMAND(ID_COLORMOOD_RETRO, &CImageProcessingDoc::OnColorMoodRetro)
	ON_COMMAND(ID_COLORMOOD_WARM, &CImageProcessingDoc::OnColorMoodWarm)
	ON_COMMAND(ID_COLORMOOD_COOL, &CImageProcessingDoc::OnColorMoodCool)
	ON_COMMAND(ID_COLORMOOD_BLOOM, &CImageProcessingDoc::OnColorMoodBloom)
	ON_COMMAND(ID_COLORMOOD_VIGNETTE, &CImageProcessingDoc::OnColorMoodVignette)

END_MESSAGE_MAP()


CImageProcessingDoc::CImageProcessingDoc()
{
	m_pImage = NULL;
}

CImageProcessingDoc::~CImageProcessingDoc()
{
	if (NULL != m_pImage)
		delete m_pImage;
}

BOOL CImageProcessingDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	m_pImage = new CxImage;
	m_pImage->Load(lpszPathName, FindType(lpszPathName));

	return TRUE;
}

BOOL CImageProcessingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}


void CImageProcessingDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}


#ifdef _DEBUG
void CImageProcessingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CImageProcessingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif 


CString CImageProcessingDoc::FindExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len - 1; i >= 0; i--) {
		if (name[i] == '.') {
			return name.Mid(i + 1);
		}
	}
	return CString(_T(""));
}

CString CImageProcessingDoc::RemoveExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len - 1; i >= 0; i--) {
		if (name[i] == '.') {
			return name.Mid(0, i);
		}
	}
	return name;
}

int CImageProcessingDoc::FindType(const CString& ext)
{
	return CxImage::GetTypeIdFromName(ext);
}

void CImageProcessingDoc::OnFileRevert()
{
	if (!m_pImage || GetPathName().IsEmpty())
	{
		AfxMessageBox(_T("Cannot revert. File path is unknown or no image is loaded."));
		return;
	}

	CString strPath = GetPathName();

	delete m_pImage;
	m_pImage = NULL;

	m_pImage = new CxImage;
	if (!m_pImage->Load(strPath, FindType(strPath)))
	{
		AfxMessageBox(_T("Error: Failed to reload the original image from disk."));
		delete m_pImage;
		m_pImage = NULL;
		return;
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::ApplyConvolution3x3(double kernel[3][3])
{
	if (!m_pImage) return;

	CxImage tempImage(*m_pImage);

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			double sumR = 0.0, sumG = 0.0, sumB = 0.0;

			for (int ky = 0; ky < 3; ky++)
			{
				for (int kx = 0; kx < 3; kx++)
				{
					RGBQUAD pixel = tempImage.GetPixelColor(x + kx - 1, y + ky - 1);

					double k = kernel[ky][kx];
					sumR += pixel.rgbRed * k;
					sumG += pixel.rgbGreen * k;
					sumB += pixel.rgbBlue * k;
				}
			}

			RGBQUAD newPixel;
			newPixel.rgbRed = Clamp(sumR);
			newPixel.rgbGreen = Clamp(sumG);
			newPixel.rgbBlue = Clamp(sumB);
			newPixel.rgbReserved = 0;

			m_pImage->SetPixelColor(x, y, newPixel);
		}
	}
}

BYTE CImageProcessingDoc::Clamp(double value)
{
	if (value > 255.0) return 255;
	if (value < 0.0) return 0;
	return (BYTE)value;
}

void CImageProcessingDoc::OnStylizeCartoon()
{
	if (!m_pImage) return;
	AfxMessageBox(_T("Cartoon Filter 실행"));
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnStylizeOilPainting()
{
	// 1. 이미지가 열려있는지 확인
	if (!m_pImage) return;

	// 컬러 이미지가 아니면 처리 불가
	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Oil Painting filter can only be applied to color images."));
		return;
	}

	// --- 필터 튜닝 값 (상수) ---
	const int radius = 4;        // 붓 터치 크기 (클수록 뭉개짐이 커짐, 2~5 추천)
	const int intensityLevels = 20; // 밝기 단계 (작을수록 색이 단순해짐, 10~30 추천)
	// --------------------------

	// 2. 원본 이미지를 복사 (계산 중 원본 데이터 참조용)
	CxImage tempImage(*m_pImage);

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	// 3. 전체 픽셀 순회
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 각 밝기 레벨별로 카운트할 배열과 색상 합계 배열 초기화
			// intensityLevels 개수만큼 0으로 초기화
			int intensityCount[256] = { 0 };
			int sumR[256] = { 0 };
			int sumG[256] = { 0 };
			int sumB[256] = { 0 };

			// 4. 커널(주변 픽셀) 순회 (-radius ~ +radius)
			for (int ky = -radius; ky <= radius; ky++)
			{
				for (int kx = -radius; kx <= radius; kx++)
				{
					int nx = x + kx;
					int ny = y + ky;

					// 이미지 범위를 벗어나지 않도록 체크
					if (nx >= 0 && nx < width && ny >= 0 && ny < height)
					{
						RGBQUAD pixel = tempImage.GetPixelColor(nx, ny);

						// 현재 픽셀의 밝기(Intensity) 계산 (0~255)
						int currentIntensity = (pixel.rgbRed + pixel.rgbGreen + pixel.rgbBlue) / 3;

						// 밝기를 intensityLevels 단계로 양자화(Quantization)
						// 예: 255를 20단계로 나누면, 밝기 0~12는 0번 방, 13~25는 1번 방...
						int binIndex = (currentIntensity * intensityLevels) / 255;

						// 인덱스 안전 장치
						if (binIndex >= intensityLevels) binIndex = intensityLevels - 1;

						// 해당 밝기 단계에 카운트 증가 및 색상 누적
						intensityCount[binIndex]++;
						sumR[binIndex] += pixel.rgbRed;
						sumG[binIndex] += pixel.rgbGreen;
						sumB[binIndex] += pixel.rgbBlue;
					}
				}
			}

			// 5. 가장 빈도수가 높은(픽셀이 제일 많은) 밝기 단계 찾기
			int maxCount = -1;
			int maxIndex = 0;

			for (int i = 0; i < intensityLevels; i++)
			{
				if (intensityCount[i] > maxCount)
				{
					maxCount = intensityCount[i];
					maxIndex = i;
				}
			}

			// 6. 가장 많이 나온 단계의 '평균 색상' 계산
			RGBQUAD newPixel;
			if (maxCount > 0) // 0으로 나누기 방지
			{
				newPixel.rgbRed = (BYTE)(sumR[maxIndex] / maxCount);
				newPixel.rgbGreen = (BYTE)(sumG[maxIndex] / maxCount);
				newPixel.rgbBlue = (BYTE)(sumB[maxIndex] / maxCount);
				newPixel.rgbReserved = 0;
			}
			else
			{
				// 예외 상황: 원래 색 유지
				newPixel = tempImage.GetPixelColor(x, y);
			}

			// 7. 결과 이미지에 적용
			m_pImage->SetPixelColor(x, y, newPixel);
		}
	}

	// 8. 화면 갱신
	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnStylizePopArt()
{
	const int nLevels = 4;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Pop Art filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	const double stepSize = 256.0 / nLevels;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			int binIndex_R = (int)(color.rgbRed / stepSize);
			double newRed = (binIndex_R * stepSize) + (stepSize / 2.0);

			int binIndex_G = (int)(color.rgbGreen / stepSize);
			double newGreen = (binIndex_G * stepSize) + (stepSize / 2.0);

			int binIndex_B = (int)(color.rgbBlue / stepSize);
			double newBlue = (binIndex_B * stepSize) + (stepSize / 2.0);


			color.rgbRed = (BYTE)std::min(255.0, std::max(0.0, newRed));
			color.rgbGreen = (BYTE)std::min(255.0, std::max(0.0, newGreen));
			color.rgbBlue = (BYTE)std::min(255.0, std::max(0.0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnStylizeEmboss()
{
	if (!m_pImage) return;

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	CxImage I_Gray(*m_pImage);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD color = I_Gray.GetPixelColor(x, y);

			int grayValue = static_cast<int>(0.299 * color.rgbRed + 0.587 * color.rgbGreen + 0.114 * color.rgbBlue);

			color.rgbRed = color.rgbGreen = color.rgbBlue = Clamp(grayValue);

			I_Gray.SetPixelColor(x, y, color);
		}
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD gray_pixel = I_Gray.GetPixelColor(x, y);
			m_pImage->SetPixelColor(x, y, gray_pixel);
		}
	}

	double kernel_emboss[3][3] = {
		{-2.0, -1.0, 0.0},
		{-1.0, 1.0, 1.0},
		{ 0.0, 1.0, 2.0}
	};

	ApplyConvolution3x3(kernel_emboss);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			double embossed_value = color.rgbRed;

			int final_gray_value = Clamp(embossed_value);

			RGBQUAD final_pixel;
			final_pixel.rgbRed = final_gray_value;
			final_pixel.rgbGreen = final_gray_value;
			final_pixel.rgbBlue = final_gray_value;
			final_pixel.rgbReserved = 0;

			m_pImage->SetPixelColor(x, y, final_pixel);
		}
	}

	UpdateAllViews(NULL);
}


void CImageProcessingDoc::OnColorMoodBloom()
{
	if (!m_pImage) return;

	const float strength = 0.7f;
	const int threshold = 0;

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	CxImage original_copy(*m_pImage);

	CxImage I_Bright(*m_pImage);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD color = I_Bright.GetPixelColor(x, y);
			int L = (color.rgbRed + color.rgbGreen + color.rgbBlue) / 3;

			if (L < threshold) {
				color.rgbRed = color.rgbGreen = color.rgbBlue = 0;
				I_Bright.SetPixelColor(x, y, color);
			}
		}
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD bright_pixel = I_Bright.GetPixelColor(x, y);
			m_pImage->SetPixelColor(x, y, bright_pixel);
		}
	}

	double kernel_gaussian[3][3] = {
		{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
		{2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0},
		{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}
	};

	ApplyConvolution3x3(kernel_gaussian);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBQUAD blur_color = m_pImage->GetPixelColor(x, y);
			RGBQUAD original_color = original_copy.GetPixelColor(x, y);

			RGBQUAD final_color;
			final_color.rgbRed = Clamp(original_color.rgbRed + (int)(blur_color.rgbRed * strength));
			final_color.rgbGreen = Clamp(original_color.rgbGreen + (int)(blur_color.rgbGreen * strength));
			final_color.rgbBlue = Clamp(original_color.rgbBlue + (int)(blur_color.rgbBlue * strength));
			final_color.rgbReserved = 0;

			m_pImage->SetPixelColor(x, y, final_color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodRetro()
{
	const double SEP_R_R = 0.393; const double SEP_R_G = 0.769; const double SEP_R_B = 0.189;
	const double SEP_G_R = 0.349; const double SEP_G_G = 0.686; const double SEP_G_B = 0.168;
	const double SEP_B_R = 0.272; const double SEP_B_G = 0.534; const double SEP_B_B = 0.131;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Retro filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			double originalRed = color.rgbRed;
			double originalGreen = color.rgbGreen;
			double originalBlue = color.rgbBlue;

			double newRed = (originalRed * SEP_R_R) + (originalGreen * SEP_R_G) + (originalBlue * SEP_R_B);
			double newGreen = (originalRed * SEP_G_R) + (originalGreen * SEP_G_G) + (originalBlue * SEP_B_B);
			double newBlue = (originalRed * SEP_B_R) + (originalGreen * SEP_B_G) + (originalBlue * SEP_B_B);

			color.rgbRed = (BYTE)std::min(255.0, std::max(0.0, newRed));
			color.rgbGreen = (BYTE)std::min(255.0, std::max(0.0, newGreen));
			color.rgbBlue = (BYTE)std::min(255.0, std::max(0.0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodWarm()
{
	const int nRedAdjust = 25;
	const int nGreenAdjust = 10;
	const int nBlueAdjust = -15;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Warm filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			int newRed = color.rgbRed + nRedAdjust;
			int newGreen = color.rgbGreen + nGreenAdjust;
			int newBlue = color.rgbBlue + nBlueAdjust;

			color.rgbRed = (BYTE)std::min(255, std::max(0, newRed));
			color.rgbGreen = (BYTE)std::min(255, std::max(0, newGreen));
			color.rgbBlue = (BYTE)std::min(255, std::max(0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodCool()
{
	const int nRedAdjust = -15;
	const int nGreenAdjust = -10;
	const int nBlueAdjust = 25;

	if (!m_pImage) return;

	if (m_pImage->GetBpp() < 24)
	{
		AfxMessageBox(_T("Cool filter can only be applied to color images."));
		return;
	}

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			int newRed = color.rgbRed + nRedAdjust;
			int newGreen = color.rgbGreen + nGreenAdjust;
			int newBlue = color.rgbBlue + nBlueAdjust;

			color.rgbRed = (BYTE)std::min(255, std::max(0, newRed));
			color.rgbGreen = (BYTE)std::min(255, std::max(0, newGreen));
			color.rgbBlue = (BYTE)std::min(255, std::max(0, newBlue));

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}

void CImageProcessingDoc::OnColorMoodVignette()
{
	const double VIGNETTE_STRENGTH = 1.3;

	if (!m_pImage) return;

	int width = m_pImage->GetWidth();
	int height = m_pImage->GetHeight();

	double centerX = width / 2.0;
	double centerY = height / 2.0;

	double maxDistance = sqrt((centerX * centerX) + (centerY * centerY));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			double currentDistance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));

			double ratio = (currentDistance / maxDistance) * VIGNETTE_STRENGTH;

			double brightnessFactor = 1.0 - ratio;

			brightnessFactor = std::min(1.0, std::max(0.0, brightnessFactor));

			RGBQUAD color = m_pImage->GetPixelColor(x, y);

			color.rgbRed = (BYTE)(color.rgbRed * brightnessFactor);
			color.rgbGreen = (BYTE)(color.rgbGreen * brightnessFactor);
			color.rgbBlue = (BYTE)(color.rgbBlue * brightnessFactor);

			m_pImage->SetPixelColor(x, y, color);
		}
	}

	UpdateAllViews(NULL);
}
