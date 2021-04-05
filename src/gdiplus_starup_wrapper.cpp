#include "gdiplus_starup_wrapper.h"

PaintLite::GdiplusStarupWrapper::GdiplusStarupWrapper( ULONG_PTR token ) :
    startupInput{},
    m_token{ token }
{
    Gdiplus::GdiplusStartup( &m_token, &startupInput, nullptr );
}

PaintLite::GdiplusStarupWrapper::~GdiplusStarupWrapper()
{
    Gdiplus::GdiplusShutdown( m_token );
}
