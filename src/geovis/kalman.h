#ifndef RCR_LEVEL1PAYLOAD_KALMAN_H_
#define RCR_LEVEL1PAYLOAD_KALMAN_H_

namespace rcr {
namespace level1payload {

struct kalman_t {
  double process_noise;     // process noise covariance
  double measurement_noise; // measurement noise covariance
  double value;             // value
  double error;             // estimation error covariance
  double gain;              // kalman gain
};

} // namespace level1_payload
} // namespace rcr

#endif // RCR_LEVEL1PAYLOAD_KALMAN_H_