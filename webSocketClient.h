#include <websocketpp/config/asio_no_tls_client.hpp>  // TLS 없이 WebSocket 사용
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <boost/optional.hpp>

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio_client> client;  // TLS 미사용 클라이언트 설정

class WebSocketClient {
public:
    WebSocketClient(const std::string& uri) : connected(false) {
        c.init_asio();  // WebSocket 초기화

        // 메시지 핸들러 등록
        c.set_message_handler([this](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            std::cout << "Received: " << msg->get_payload() << std::endl;
        });

        // 연결 성공 핸들러 등록
        c.set_open_handler([this](websocketpp::connection_hdl hdl) {
            std::cout << "WebSocket connection opened." << std::endl;
            this->hdl = hdl;
            connected = true;  // 연결 상태 플래그 설정
        });

        // 연결 해제 핸들러 등록
        c.set_close_handler([this](websocketpp::connection_hdl) {
            std::cout << "WebSocket connection closed." << std::endl;
            connected = false;  // 연결 상태 플래그 해제
        });

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);  // WebSocket 연결 생성

        if (ec) {
            std::cerr << "Connection error: " << ec.message() << std::endl;
            return;
        }

        hdl = con->get_handle();  // 연결 핸들 저장
        c.connect(con);  // WebSocket 서버에 연결

        c.clear_access_channels(websocketpp::log::alevel::all);  // 모든 로그 비활성화
        c.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect);  // 필요한 로그만 설정
        // WebSocket 클라이언트 실행을 별도의 스레드에서 처리
        thread = std::thread([this]() { c.run(); });
    }

    ~WebSocketClient() {
        c.stop();
        if (thread.joinable()) {
            thread.join();
        }
    }

    // 연결된 상태에서 차량 데이터를 서버로 전송
   // 차량 데이터를 서버로 전송
void sendCarData(const std::string& uni_num, double lat, double lon, double braking_distance, 
                 boost::optional<bool> car_flag = boost::none) {  // car_flag는 선택적

    if (!connected) {
        std::cerr << "Send error: WebSocket not connected" << std::endl;
        return;
    }

    json j = {
        {"uni_num", uni_num},
        {"car_lat", lat},
        {"car_lon", lon},
        {"braking_distance", braking_distance}
    };

    // car_flag가 전달된 경우만 JSON에 추가
    if (car_flag.has_value()) {
        j["car_flag"] = car_flag.value();
    }

    websocketpp::lib::error_code ec;
    c.send(hdl, j.dump(), websocketpp::frame::opcode::text, ec);  // 데이터 전송

    if (ec) {
        std::cerr << "Send error: " << ec.message() << std::endl;
    }

    
}

private:
    client c;
    websocketpp::connection_hdl hdl;
    std::thread thread;
    std::atomic<bool> connected;  // 연결 상태 플래그
};
