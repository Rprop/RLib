/********************************************************************
	Created:	2014/07/31  20:09
	Filename: 	RLib_TextSpeaker.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_String.h>

#if !(defined _USE_TEXTSPEAKER) && !(defined _DISABLE_TEXTSPEAKER)
#define _USE_TEXTSPEAKER

namespace System 
{
	/// <summary>
	/// The Microsoft namespaces provide classes that provided by the operating system
	/// </summary>
	namespace Microsoft
	{
		/// <summary>
		/// Simple class to speak some text
		/// </summary>
		class RLIB_API TextSpeaker
		{
		private:
			LPVOID m_data;
		public:
			TextSpeaker();
			~TextSpeaker();
			RLIB_DECLARE_DYNCREATE;
		public:
			/// <summary>
			/// Speaks some text
			/// </summary>
			void Speak(String text, bool async = true);
			/// <summary>
			/// Pauses speaking
			/// </summary>
			void Pause();
			/// <summary>
			/// Resumes speaking
			/// </summary>
			void Resume();
			/// <summary>
			/// Stops speaking
			/// </summary>
			void Stop();
			/// <summary>
			/// Sepeaks some text, and saves to wav
			/// </summary>
			void SpeakToWav(String text, String wavfile, bool async = true);
		};
	}
} // namespace System


#endif // _USE_TEXTSPEAKER
