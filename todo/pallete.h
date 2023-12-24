/// @brief style colour index
typedef enum style_t
{
    /// backtrace formatting

    /// @brief the colour of the backtrace frame index
    eStyleFrameIndex,

    /// @brief the colour of the backtrace frame address
    eStyleFrameAddress,

    /// @brief the colour of the backtrace frame function name
    eStyleFrameSymbol,

    /// @brief the colour of the backtrace frame recursion count
    eStyleFrameRecursion,

    /// @brief the colour of the backtrace frame source file
    eStyleFrameFile,

    /// @brief the colour of the backtrace frame source text
    eStyleFrameSource,

    /// notify formatting

    /// @brief associated colour for an info message
    eStyleMsgInfo,
    eStyleInfoDebug,
    eStyleInfoWarn,
    eStyleInfoFatal,
    eStyleInfoInternal,
    eStyleInfoSorry,

    eStyleCount
} style_t;

// all colours
typedef struct pallete_t
{
    colour_t pallete[eStyleCount];
} pallete_t;
