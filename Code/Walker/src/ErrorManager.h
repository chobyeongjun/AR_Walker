/**
 * @file ErrorManager.h
 * @author Chancelor Cuddeback
 * @brief Checks for errors and runs error handlers. Places errors in a queue.
 *
 */

#ifndef ERROR_MANAGER_H
#define ERROR_MANAGER_H
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "error_types.h"
#include "error_codes.h"
#include "error_map.h"
#include <queue>

/**
 * @brief Class manages the calling of error handlers and triggers. Only the control MCU is required to use this.
 * The triggers and handlers must be assigned before check is ran. They are defined in error_triggers.h and error_handlers.h.
 * Check should be called every loop, and should not, severely, increase the loop time.
 */
class ErrorManager
{
public:
    ErrorManager() {};

    /**
     * @brief Runs the error manager. This should be called every loop.
     *
     * @return true No errors
     * @return false
     */
    template <typename data>
    bool run(data *_data)
    {
        // Check for errors
        for (int i_error = (NO_ERROR + 1); i_error != ERROR_CODE_LENGTH; i_error++)
        {
            // Current error code
            ErrorCodes error_code = static_cast<ErrorCodes>(i_error); // ErrorCodes 가 ESR_ERROR, MOTOR_ERROR 등으로 되어있음

            // Get error type
            ErrorType *error = error_map.at(error_code);

            // Check for error
            if (error->check(_data))
            {
                error->handle(_data);
                this->_pushError(error_code);
            }
        }

        return static_cast<bool>(this->errorQueueSize());
    }

    /**
     * @brief Check the size of the error queue
     *
     * @return int
     */
    int errorQueueSize()
    {
        return this->_error_queue.size();
    }

    /**
     * @brief Get the next error code in the queue
     *
     * @return ErrorCodes
     */
    ErrorCodes popError()
    {
        ErrorCodes error_code = this->_error_queue.front(); // front() : 맨 앞의 요소를 확인
        this->_error_queue.pop();                           // pop() : 맨 앞의 요소를 제거
        return error_code;
    }

private:
    std::queue<ErrorCodes> _error_queue; // _error_queue를 만들건데, ErrorCodes 타입만 담을 거고, 에러 코드들을 순서대로 저장하는 큐(queue) 자료구조

    /**
     * @brief Pushes an error code to the queue
     *
     * @param error_code
     */
    void _pushError(ErrorCodes error_code)
    {
        this->_error_queue.push(error_code);
    }
};

#endif
#endif // ERROR_MANAGER_H