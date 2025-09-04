function force = voltage_to_force_right(voltage)
% VOLTAGE_TO_FORCE_RIGHT - 오른쪽 로드셀 전압을 힘으로 변환
% 입력: voltage (V)
% 출력: force (N)
% 캘리브레이션 날짜: 03-Sep-2025 16:39:03
%
% 캘리브레이션 방정식: Force = 541.117950 * Voltage + (-654.415347)

    force = 541.117950 * voltage + (-654.415347);
end
