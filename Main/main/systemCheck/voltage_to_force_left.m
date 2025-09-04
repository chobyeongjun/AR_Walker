function force = voltage_to_force_left(voltage)
% VOLTAGE_TO_FORCE_LEFT - 왼쪽 로드셀 전압을 힘으로 변환
% 입력: voltage (V)
% 출력: force (N)
% 캘리브레이션 날짜: 03-Sep-2025 16:39:03
%
% 캘리브레이션 방정식: Force = 560.749025 * Voltage + (-682.835731)

    force = 560.749025 * voltage + (-682.835731);
end
