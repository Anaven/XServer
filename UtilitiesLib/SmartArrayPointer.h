#ifndef __SmartArrayPointer__
#define __SmartArrayPointer__

template <class T>
class SmartArrayPointer
{
    public:
        
        SmartArrayPointer(T* victim) : fT(victim)  {}
        ~SmartArrayPointer() { delete [] fT; }
        
        void SetObject(T* victim) 
        { 
            //can't use a normal assert here because "Assert.h" eventually includes this file....
            #ifdef ASSERT
                //char s[65]; 
                if (fT != NULL) qtss_printf ("_Assert: StSmartArrayPointer::SetObject() %s, %d\n", __FILE__, __LINE__); 
            #endif 

            fT = victim; 
        }
        
        T* GetObject() { return fT; }
        
        operator T*() { return fT; }
    
    private:
    
        T* fT;
};

typedef SmartArrayPointer<char> CharArrayDeleter;
#endif
