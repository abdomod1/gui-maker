#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <string>
#include <memory>

enum class PieceType {
    None,
    Pawn,
    Rook,
    Knight,
    Bishop,
    Queen,
    King
};

enum class PieceColor {
    White,
    Black,
    None
};

struct Piece {
    PieceType type = PieceType::None;
    PieceColor color = PieceColor::None;
    bool hasMoved = false;
};

class ChessGame {
public:
    ChessGame() {
        resetBoard();
    }

    void resetBoard() {
        // Initialize empty board
        board.fill({PieceType::None, PieceColor::None});

        // Set up pawns
        for (int i = 0; i < 8; ++i) {
            board[8 + i] = {PieceType::Pawn, PieceColor::Black};
            board[48 + i] = {PieceType::Pawn, PieceColor::White};
        }

        // Set up other pieces (black)
        board[0] = {PieceType::Rook, PieceColor::Black};
        board[7] = {PieceType::Rook, PieceColor::Black};
        board[1] = {PieceType::Knight, PieceColor::Black};
        board[6] = {PieceType::Knight, PieceColor::Black};
        board[2] = {PieceType::Bishop, PieceColor::Black};
        board[5] = {PieceType::Bishop, PieceColor::Black};
        board[3] = {PieceType::Queen, PieceColor::Black};
        board[4] = {PieceType::King, PieceColor::Black};

        // Set up other pieces (white)
        board[56] = {PieceType::Rook, PieceColor::White};
        board[63] = {PieceType::Rook, PieceColor::White};
        board[57] = {PieceType::Knight, PieceColor::White};
        board[62] = {PieceType::Knight, PieceColor::White};
        board[58] = {PieceType::Bishop, PieceColor::White};
        board[61] = {PieceType::Bishop, PieceColor::White};
        board[59] = {PieceType::Queen, PieceColor::White};
        board[60] = {PieceType::King, PieceColor::White};

        currentTurn = PieceColor::White;
        selectedPiece = -1;
        gameOver = false;
    }

    void drawBoard() {
        ImGui::Begin("Chess Board", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        const float squareSize = 60.0f;
        const ImVec2 boardSize(squareSize * 8, squareSize * 8);
        const ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        // Draw board squares
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                int index = y * 8 + x;
                ImVec2 pos(cursorPos.x + x * squareSize, cursorPos.y + y * squareSize);
                ImVec2 posEnd(pos.x + squareSize, pos.y + squareSize);

                // Square color
                bool isLight = (x + y) % 2 == 0;
                ImU32 squareColor = isLight ? IM_COL32(240, 217, 181, 255) : IM_COL32(181, 136, 99, 255);

                // Highlight selected square
                if (index == selectedPiece) {
                    squareColor = IM_COL32(247, 247, 105, 255);
                }

                // Highlight possible moves
                if (!possibleMoves.empty() && std::find(possibleMoves.begin(), possibleMoves.end(), index) != possibleMoves.end()) {
                    squareColor = IM_COL32(247, 105, 105, 255);
                }

                ImGui::GetWindowDrawList()->AddRectFilled(pos, posEnd, squareColor);

                // Draw piece
                if (board[index].type != PieceType::None) {
                    std::string pieceSymbol = getPieceSymbol(board[index]);
                    ImVec2 textSize = ImGui::CalcTextSize(pieceSymbol.c_str());
                    ImVec2 textPos(pos.x + (squareSize - textSize.x) * 0.5f, pos.y + (squareSize - textSize.y) * 0.5f);
                    ImU32 pieceColor = board[index].color == PieceColor::White ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255);
                    ImGui::GetWindowDrawList()->AddText(textPos, pieceColor, pieceSymbol.c_str());
                }

                // Handle click
                if (ImGui::IsMouseHoveringRect(pos, posEnd) && ImGui::IsMouseClicked(0) && !gameOver) {
                    handleSquareClick(index);
                }
            }
        }

        ImGui::SetWindowSize(boardSize);
        ImGui::End();

        // Draw game status
        ImGui::Begin("Game Status");
        if (gameOver) {
            ImGui::Text("Game Over! %s wins!", winner == PieceColor::White ? "White" : "Black");
        } else {
            ImGui::Text("Current Turn: %s", currentTurn == PieceColor::White ? "White" : "Black");
        }
        
        if (ImGui::Button("Reset Game")) {
            resetBoard();
        }
        ImGui::End();
    }

private:
    std::array<Piece, 64> board;
    PieceColor currentTurn;
    int selectedPiece;
    std::vector<int> possibleMoves;
    bool gameOver;
    PieceColor winner;

    std::string getPieceSymbol(const Piece& piece) {
        if (piece.type == PieceType::None) return "";

        switch (piece.type) {
            case PieceType::King:   return piece.color == PieceColor::White ? "♔" : "♚";
            case PieceType::Queen:  return piece.color == PieceColor::White ? "♕" : "♛";
            case PieceType::Rook:   return piece.color == PieceColor::White ? "♖" : "♜";
            case PieceType::Bishop: return piece.color == PieceColor::White ? "♗" : "♝";
            case PieceType::Knight: return piece.color == PieceColor::White ? "♘" : "♞";
            case PieceType::Pawn:  return piece.color == PieceColor::White ? "♙" : "♟";
            default: return "";
        }
    }

    void handleSquareClick(int index) {
        // If no piece is selected and the clicked square has a piece of current turn's color
        if (selectedPiece == -1 && board[index].color == currentTurn) {
            selectedPiece = index;
            calculatePossibleMoves(index);
            return;
        }

        // If a piece is already selected
        if (selectedPiece != -1) {
            // If clicking on a possible move square
            if (std::find(possibleMoves.begin(), possibleMoves.end(), index) != possibleMoves.end()) {
                movePiece(selectedPiece, index);
            }
            
            // Deselect if clicking on another piece of same color
            if (board[index].color == currentTurn) {
                selectedPiece = index;
                calculatePossibleMoves(index);
            } else {
                selectedPiece = -1;
                possibleMoves.clear();
            }
        }
    }

    void movePiece(int from, int to) {
        // Mark piece as moved (for pawns, kings, rooks)
        board[from].hasMoved = true;

        // Check if this is a capture
        if (board[to].type != PieceType::None) {
            // Check if it's the king (game over)
            if (board[to].type == PieceType::King) {
                gameOver = true;
                winner = currentTurn;
            }
        }

        // Move the piece
        board[to] = board[from];
        board[from] = {PieceType::None, PieceColor::None};

        // Switch turns
        currentTurn = (currentTurn == PieceColor::White) ? PieceColor::Black : PieceColor::White;

        // Clear selection
        selectedPiece = -1;
        possibleMoves.clear();
    }

    void calculatePossibleMoves(int index) {
        possibleMoves.clear();
        const Piece& piece = board[index];
        int x = index % 8;
        int y = index / 8;

        switch (piece.type) {
            case PieceType::Pawn:
                calculatePawnMoves(x, y, piece.color);
                break;
            case PieceType::Rook:
                calculateRookMoves(x, y, piece.color);
                break;
            case PieceType::Knight:
                calculateKnightMoves(x, y, piece.color);
                break;
            case PieceType::Bishop:
                calculateBishopMoves(x, y, piece.color);
                break;
            case PieceType::Queen:
                calculateRookMoves(x, y, piece.color);
                calculateBishopMoves(x, y, piece.color);
                break;
            case PieceType::King:
                calculateKingMoves(x, y, piece.color);
                break;
            default:
                break;
        }
    }

    void calculatePawnMoves(int x, int y, PieceColor color) {
        int direction = (color == PieceColor::White) ? -1 : 1;
        int startRow = (color == PieceColor::White) ? 6 : 1;

        // Move forward one square
        if (isValidSquare(x, y + direction) && board[(y + direction) * 8 + x].type == PieceType::None) {
            possibleMoves.push_back((y + direction) * 8 + x);

            // Move forward two squares from starting position
            if (y == startRow && board[(y + 2 * direction) * 8 + x].type == PieceType::None) {
                possibleMoves.push_back((y + 2 * direction) * 8 + x);
            }
        }

        // Capture diagonally
        for (int dx : {-1, 1}) {
            if (isValidSquare(x + dx, y + direction)) {
                int targetIndex = (y + direction) * 8 + (x + dx);
                if (board[targetIndex].type != PieceType::None && board[targetIndex].color != color) {
                    possibleMoves.push_back(targetIndex);
                }
            }
        }
    }

    void calculateRookMoves(int x, int y, PieceColor color) {
        // Horizontal and vertical moves
        const std::array<std::pair<int, int>, 4> directions = {
            {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
        };

        for (const auto& dir : directions) {
            for (int i = 1; i < 8; ++i) {
                int nx = x + i * dir.first;
                int ny = y + i * dir.second;

                if (!isValidSquare(nx, ny)) break;

                int index = ny * 8 + nx;
                if (board[index].type == PieceType::None) {
                    possibleMoves.push_back(index);
                } else {
                    if (board[index].color != color) {
                        possibleMoves.push_back(index);
                    }
                    break;
                }
            }
        }
    }

    void calculateKnightMoves(int x, int y, PieceColor color) {
        const std::array<std::pair<int, int>, 8> moves = {
            {{2, 1}, {2, -1}, {-2, 1}, {-2, -1},
            {1, 2}, {1, -2}, {-1, 2}, {-1, -2}}
        };

        for (const auto& move : moves) {
            int nx = x + move.first;
            int ny = y + move.second;

            if (isValidSquare(nx, ny)) {
                int index = ny * 8 + nx;
                if (board[index].type == PieceType::None || board[index].color != color) {
                    possibleMoves.push_back(index);
                }
            }
        }
    }

    void calculateBishopMoves(int x, int y, PieceColor color) {
        // Diagonal moves
        const std::array<std::pair<int, int>, 4> directions = {
            {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}
        };

        for (const auto& dir : directions) {
            for (int i = 1; i < 8; ++i) {
                int nx = x + i * dir.first;
                int ny = y + i * dir.second;

                if (!isValidSquare(nx, ny)) break;

                int index = ny * 8 + nx;
                if (board[index].type == PieceType::None) {
                    possibleMoves.push_back(index);
                } else {
                    if (board[index].color != color) {
                        possibleMoves.push_back(index);
                    }
                    break;
                }
            }
        }
    }

    void calculateKingMoves(int x, int y, PieceColor color) {
        // All adjacent squares
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;

                int nx = x + dx;
                int ny = y + dy;

                if (isValidSquare(nx, ny)) {
                    int index = ny * 8 + nx;
                    if (board[index].type == PieceType::None || board[index].color != color) {
                        possibleMoves.push_back(index);
                    }
                }
            }
        }
    }

    bool isValidSquare(int x, int y) {
        return x >= 0 && x < 8 && y >= 0 && y < 8;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Chess Game");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    ChessGame game;

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // Clear screen
        window.clear(sf::Color(50, 50, 50));

        // Draw chess board
        game.drawBoard();

        // Render ImGui
        ImGui::SFML::Render(window);

        // Display window
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}