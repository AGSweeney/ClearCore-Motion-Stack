// EXCERPT — source: ProjectTemplate/libClearCore/inc/CoordinatedMotionController.h
// EVIDENCE: E1 | symbol: CoordinatedMotionController | lines: 44-90
/**
    \class CoordinatedMotionController
    \brief Controller for coordinated motion between two motors

    This class provides high-level control for coordinated arc motion,
    managing arc queues and integrating with the motor control system.
**/
class CoordinatedMotionController {
public:
    /**
        \brief Constructor
    **/
    CoordinatedMotionController();

    /**
        \brief Initialize with two motor references
        
        \param[in] motorX Pointer to X-axis motor
        \param[in] motorY Pointer to Y-axis motor
        
        \return true if initialization successful
    **/
    bool Initialize(MotorDriver* motorX, MotorDriver* motorY);

    /**
        \brief Issue a single arc move
        
        \param[in] centerX Arc center X position in steps
        \param[in] centerY Arc center Y position in steps
        \param[in] radius Arc radius in steps
        \param[in] startAngle Start angle in radians
        \param[in] endAngle End angle in radians
        \param[in] clockwise Direction (true = clockwise)
        
        \return true if arc command accepted
    **/
    bool MoveArc(int32_t centerX, int32_t centerY,
                 int32_t radius,
                 double startAngle, double endAngle,
                 bool clockwise);

    /**
        \brief Issue a continuous arc (chains from current position)
        
        \param[in] centerX Arc center X position in steps
        \param[in] centerY Arc center Y position in steps
        \param[in] radius Arc radius in steps
