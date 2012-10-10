#include <stdlib.h>

namespace Utility
{

    bool Compare(float First, float Second, int ComparisonType)
    {
        switch(ComparisonType)
        {
            case 0:
                return First == Second;

            case 1:
                return First != Second;

            case 2:
                return First > Second;

            case 3:
                return First < Second;

            case 4:
                return First >= Second;

            case 5:
                return First <= Second;

            default:
                return false;
        };
    }


    void SetRandomSeed(int Seed)
    {
        srand(Seed);
    }

    void SetRandomSeedToTimer()
    {
        srand(GetTickCount());
    }

    // Useful Functions
    int Round(float Value)
    {
        return (Value > 0) ? (int)floor(Value + 0.5f) : (int)ceil(Value - 0.5f);
    }

    // Float Expressions
    float GenerateRandom(float Minimum, float Maximum)
    {
        return ((Maximum-Minimum)*((float)rand()/RAND_MAX))+Minimum;
    }
    float Limit(float Value, float Minimum, float Maximum)
    {
        if (Minimum < Maximum)
            return (Value < Minimum) ? (Minimum) : (Value > Maximum ? Maximum : Value);
            return (Value < Maximum) ? (Maximum) : (Value > Minimum ? Minimum : Value);
    }


    float Nearest(float Value, float Minimum, float Maximum)
    {
        return ((Minimum > Value) ? (Minimum - Value) : (Value - Minimum)) > 
            ((Maximum > Value) ? (Maximum - Value) : (Value - Maximum)) ?
            Maximum : Minimum;
    }

    float Normalise(float Value, float Minimum, float Maximum, int LimitRange)
    {
        Value = (Value - Minimum) / (Maximum - Minimum);

        if (LimitRange != 0)
            return Limit(Value,0,1);
        return Value;
    }

    float ModifyRange(float Value, float Minimum, float Maximum, float NewMinimum, float NewMaximum, int LimitRange)
    {
        Value = NewMinimum + (Value - Minimum) * (NewMaximum - NewMinimum) / (Maximum - Minimum);

        if(LimitRange != 0) return Limit(Value,NewMinimum,NewMaximum);
        return Value;
    }

    float EuclideanMod(float Dividend, float Divisor)
    {
        return fmod((fmod(Dividend,Divisor)+Divisor),Divisor);
    }

    float UberMod(float Dividend, float Lower, float Upper)
    {
        return ModifyRange(EuclideanMod(Normalise(Dividend,Lower,Upper,0),1),0,1,Lower,Upper,0);
    }

    float Interpolate(float Value, float From, float To, int LimitRange)
    {
        Value = From+Value*(To-From);

        if(LimitRange != 0) return Limit(Value,From,To);
        return Value;
    }

    float Mirror(float Value, float From, float To)
    {
        if (From < To) {
            return From+fabs(EuclideanMod(Value-To,(To-From)*2)-(To-From));
        } else {
            return To+fabs(EuclideanMod(Value-From,(From-To)*2)-(From-To));
        }
    }

    float Wave(int Waveform, float Value, float CycleStart, float CycleEnd, float Minimum, float Maximum)
    {
        switch(Waveform)
        {
            case 0:
            {
                // Sine
                return ModifyRange(sin(ModifyRange(Value,CycleStart,CycleEnd,0,6.283185307179586476925286766559f,0)),-1,1,Minimum,Maximum,0);
            }

            case 1:
            {
                // Cosine
                return ModifyRange(cos(ModifyRange(Value,CycleStart,CycleEnd,0,6.283185307179586476925286766559f,0)),-1,1,Minimum,Maximum,0);
            }

            case 2:
            {
                // Saw
                return UberMod(ModifyRange(Value, CycleStart, CycleEnd, Minimum, Maximum, 0), Minimum, Maximum);
            }

            case 3:
            {
                // Inverted Saw
                return UberMod(ModifyRange(Value, CycleStart, CycleEnd, Maximum, Minimum, 0), Minimum, Maximum);
            }

            case 4:
            {
                // Triangle
                return Mirror(ModifyRange(Value, CycleStart, CycleStart+(CycleEnd-CycleStart)/2, Minimum, Maximum, 0), Minimum, Maximum);
            }

            case 5:
            {
                // Square
                if (UberMod(Value, CycleStart, CycleEnd) < CycleStart+(CycleEnd-CycleStart)/2) return Minimum; else return Maximum;
            }

            default:
            {            
                // Non-existing waveform
                return 0;
            }
        };
    }

    float ExpressionCompare(float First, float Second, int ComparisonType, float ReturnIfTrue, float ReturnIfFalse)
    {
        if (Compare(First,Second,ComparisonType)) return ReturnIfTrue; else return ReturnIfFalse;
    }

    float Approach(float Value, float Amount, float Target)
    {
        return (Value<Target) ? min(Value + Amount, Target) : max(Value - Amount, Target);
    }

    // Integer versions of the float expressions
    int IntGenerateRandom(float Minimum, float Maximum)
    {
        return Round(GenerateRandom(Minimum,Maximum));
    }


    int IntLimit(float Value, float Minimum, float Maximum)
    {
        return Round(Limit(Value,Minimum,Maximum));
    }
    int IntNearest(float Value, float Minimum, float Maximum)
    {
        return Round(Nearest(Value,Minimum,Maximum));
    }
    int IntNormalise(float Value, float Minimum, float Maximum, int LimitRange)
    {
        return Round(Normalise(Value,Minimum,Maximum,LimitRange));
    }

    int IntModifyRange(float Value, float Minimum, float Maximum, float NewMinimum, float NewMaximum, int LimitRange)
    {
        return Round(ModifyRange(Value,Minimum,Maximum,NewMinimum,NewMaximum,LimitRange));
    }

    int IntWave(int Waveform, float Value, float CycleStart, float CycleEnd, float Minimum, float Maximum)
    {
        return Round(Wave(Waveform,Value,CycleStart,CycleEnd,Minimum,Maximum));
    }
    int IntEuclideanMod(float Dividend, float Divisor)
    {
        return Round(EuclideanMod(Dividend,Divisor));
    }
    int IntUberMod(float Dividend, float Lower, float Upper)
    {
        return Round(UberMod(Dividend,Lower,Upper));
    }

    int IntInterpolate(float Value, float From, float To, int LimitRange)
    {
        return Round(Interpolate(Value,From,To,LimitRange));
    }
    int IntMirror(float Value, float From, float To)
    {
        return Round(Mirror(Value,From,To));
    }

    int IntExpressionCompare(float First, float Second, int ComparisonType, float ReturnIfTrue, float ReturnIfFalse)
    {
        return Round(ExpressionCompare(First, Second, ComparisonType, ReturnIfTrue, ReturnIfFalse));
    }

    int IntApproach(float Value, float Amount, float Target)
    {
        return Round(Approach(Value, Amount, Target));
    }

    // String expressions
    std::string Substr(std::string String, int Start, int Length)
    {
        if(Start < 0)
            Start = String.size() + Start;

        if(Length < 0)
            Length = String.size();

        return String.substr(Start, Length);
    }

    std::string StrExpressionCompare(float First, float Second, int ComparisonType, std::string ReturnIfTrue, std::string ReturnIfFalse)
    {
        if (Compare(First, Second, ComparisonType)) 
            return ReturnIfTrue; 
        else 
            return ReturnIfFalse;
    }
}