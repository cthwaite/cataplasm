/****[  CATAPLASM v0.1.0 – a small test framework ]****************************/
#pragma once
#include <exception>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

//  Suppress warnings for cataplasm::ExprShunt
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"
#pragma clang diagnostic ignored "-Wundefined-inline"
#endif

//  Utility macros
#define CONCAT(a, b) a##b
#define CONCAT_X(a, b) CONCAT(a, b)
#define LINE_UID(expr) CONCAT_X(expr, __LINE__)

#if defined(_LIBCPP_NO_EXCEPTIONS)
    #define CONCAT(...)
#endif

//----[ Test containers ]-------------------------------------------------------
#define _TEST_CASE_FN LINE_UID(TEST_CASE)
#define _SECTION_ID LINE_UID(SECTION)

#define TEST_CASE(...) \
    static void _TEST_CASE_FN();                                            \
    namespace {                                                             \
        cataplasm::BlockLoader LINE_UID(TEST_CASE_LOADER)(                  \
            cataplasm::NodeType::Block,                                     \
            cataplasm::Status::Null,                                        \
            &_TEST_CASE_FN,                                                 \
            cataplasm::NameTags{__VA_ARGS__},                               \
            __LINE__);                                                      \
    }                                                                       \
    static void _TEST_CASE_FN()

#define SECTION(...)                                                        \
    if(cataplasm::SectionLoader scope =                                     \
        cataplasm::SectionLoader(__LINE__, cataplasm::NameTags{__VA_ARGS__}))

//----[ Basic tests ]-----------------------------------------------------------
#define _NODE(expression, type, pass, fail, halt_on_fail)                   \
    try {                                                                   \
        cataplasm::ExprResult res = (cataplasm::ExprShunt() << expression); \
        auto status = res.status ? pass : fail;                             \
        cataplasm::g_TestReferee().push_node(                               \
            type, status, res.expr,                                         \
            __LINE__, #expression);                                         \
        if(halt_on_fail && (status == cataplasm::Status::Fail)) {           \
            return;                                                         \
        }                                                                   \
    } catch(...) {                                                          \
        cataplasm::g_TestReferee().push_exception(                          \
            std::current_exception(),                                       \
            cataplasm::NodeType::ThrowsUnexpected,                          \
            cataplasm::Status::Fail,                                        \
            #expression, __LINE__);                                         \
        return;                                                             \
    }

#define ENSURE(...)                                                         \
    _NODE(__VA_ARGS__, cataplasm::NodeType::Ensure,                         \
          cataplasm::Status::Succeed, cataplasm::Status::Fail, true)

#define VERIFY(...)                                                         \
    _NODE(__VA_ARGS__, cataplasm::NodeType::Verify,                         \
          cataplasm::Status::Succeed, cataplasm::Status::Fail, false)

#define FORBID(...)                                                         \
    _NODE(__VA_ARGS__, cataplasm::NodeType::Forbid,                         \
          cataplasm::Status::Fail, cataplasm::Status::Succeed, true)

#define REJECT(...)                                                         \
    _NODE(__VA_ARGS__, cataplasm::NodeType::Reject,                         \
          cataplasm::Status::Fail, cataplasm::Status::Succeed, false)


//----[ Exception-handling tests ]----------------------------------------------
#define _THROW_NODE(expr, node, result)                                     \
    cataplasm::g_TestReferee().push_exception(                              \
        std::current_exception(), node, result, expr, __LINE__)

#define THROWS(...)                                                         \
    try {                                                                   \
        __VA_ARGS__;                                                        \
        _THROW_NODE(#__VA_ARGS__,                                           \
            cataplasm::NodeType::Throws, cataplasm::Status::Fail);          \
    }                                                                       \
    catch(...) { _THROW_NODE(                                               \
        #__VA_ARGS__,                                                       \
        cataplasm::NodeType::Throws, cataplasm::Status::Succeed); }

#define NO_THROW(...)                                                       \
    try { __VA_ARGS__; }                                                    \
    catch(...) { _THROW_NODE(#__VA_ARGS__,                                  \
        cataplasm::NodeType::ThrowsUnexpected, cataplasm::Status::Fail); } \
    _THROW_NODE(#__VA_ARGS__,                                               \
        cataplasm::NodeType::NoThrow, cataplasm::Status::Succeed)

struct ThrowsAsFailure : public std::exception { };
#define THROWS_AS(expr, excep)                                              \
    try {                                                                   \
        expr;                                                               \
        throw ThrowsAsFailure();                                            \
    } catch(ThrowsAsFailure) {                                              \
        _THROW_NODE(#expr,                                                  \
            cataplasm::NodeType::ThrowsAs, cataplasm::Status::Fail);        \
    } catch(excep) {                                                        \
        _THROW_NODE(#expr,                                                  \
            cataplasm::NodeType::ThrowsAs, cataplasm::Status::Succeed);     \
    } catch(...) {                                                          \
        _THROW_NODE(#expr,                                                  \
            cataplasm::NodeType::ThrowsAs, cataplasm::Status::Fail);        \
    }                                                                       \

//----[ Special tests & Messages ]----------------------------------------------
#define FAIL(...) cataplasm::g_TestReferee().push_node(                     \
        cataplasm::NodeType::Fail, cataplasm::Status::Fail,                 \
        #__VA_ARGS__, __LINE__)

#define PASS(...) cataplasm::g_TestReferee().push_node(                     \
        cataplasm::NodeType::Pass, cataplasm::Status::Succeed,              \
        #__VA_ARGS__, __LINE__)

#define NOTICE(...)  cataplasm::g_TestReferee().push_node(                  \
        cataplasm::NodeType::Notice, cataplasm::Status::Null,               \
        __VA_ARGS__, __LINE__)

#define WARN(...)  cataplasm::g_TestReferee().push_node(                    \
        cataplasm::NodeType::Warn, cataplasm::Status::Null,                 \
        __VA_ARGS__, __LINE__)

namespace cataplasm {
//----[ Typedefs ]--------------------------------------------------------------
    using payload_fn = void(*)();       //<! Function pointer to test case.

//----[ CLI colours ]-----------------------------------------------------------
    enum class CLIAttr {
        Reset,
        Bold,
        Red,
        Green,
        Yellow,
        Blue,
    };

    static const char *CLIAttrCodes[] = {
        "\33[0m",
        "\33[1m",
        "\33[31m",
        "\33[32m",
        "\33[33m",
        "\33[34m",
    };

//----[ NodeType ]--------------------------------------------------------------
    enum class NodeType {
        Ensure,
        Verify,
        Forbid,
        Reject,
        Throws,
        ThrowsAs,
        NoThrow,
        ThrowsUnexpected,
        ThrowsOutOfNode,
        Fail,
        Pass,
        Block,
        Section,
        Notice,
        Warn,
    };

    //! Return true if the given NodeType can hold other nodes.
    inline bool is_container(NodeType type)
    {
        return (type == NodeType::Block || type == NodeType::Section);
    }

    static constexpr const char *NodeTypeName[] {
        "ENSURE",
        "VERIFY",
        "FORBID",
        "REJECT",
        "THROWS",
        "THOWS_AS",
        "NO_THROW",
        "ThrowsUnexpected",
        "ThrowsOutOfNode",
        "FAIL",
        "PASS",
        "Block",
        "Section",
        "NOTICE",
        "WARN",
    };

    enum class Status {
        Fail,
        Succeed,
        Null
    };

    struct ExprResult {
        bool            status;
        std::string     expr;
    };

    template <typename T>
    struct TestExpression {
        template <typename StreamType, typename ValueType>
        class to_string_able {
            template <typename SomeStream, typename SomeValue>
            static constexpr auto test(int) -> decltype(std::declval<SomeStream&>() << std::declval<SomeValue>(), std::true_type());
            template <typename, typename>
            static constexpr auto test(...) -> std::false_type;
        public:
            using value = decltype(test<StreamType, ValueType>(0));
        };
        template <typename U>
        using select = typename to_string_able<std::ostringstream, U>::value;

        TestExpression(T lhs) :
            lhs_mote_{lhs}
        {
        }

        operator ExprResult() const
        {
            return ExprResult{!!(lhs_mote_), str(lhs_mote_)};
        }

        #define def_op(which)                                                           \
        template <typename U>                                                           \
        ExprResult operator which(const U &u)                                           \
        {                                                                               \
            return { (lhs_mote_ which u), str(lhs_mote_) + " " #which " " + str(u)};    \
        }

        def_op(==)
        def_op(!=)
        def_op(< )
        def_op(> )
        def_op(<=)
        def_op(>=)
    private:
        template <typename U>
        std::string str(const U &u) { return _str(u, select<U>()); }

        template <size_t N>
        std::string str(const char (&u)[N]) const { return std::string(u); }

        std::string str(const std::string &s) const { return s; }
        std::string str(const bool &b) const { return b ? "true" : "false"; }

        template <typename U>
        std::string _str(const U &u, std::true_type) const
        {
            std::ostringstream os;
            os << u;
            return os.str();
        }

        template <typename U>
        std::string _str(const U &, std::false_type) const { return "[unknown]"; }

        const T lhs_mote_;
    };

    struct ExprShunt {
        template <typename T>
        TestExpression<const T&> operator <<(const T &t)
        {
            return {t};
        }
    };

//----[ NameTags ]--------------------------------------------------------------
    struct NameTags {
        NameTags(const char *name = "Anonymous Node", const char *tags = "") :
            name{name},
            tags{tags}
        {
        }
        const char *name;   //<! Name of the block.
        const char *tags;   //<! Tags for the block.
    };

//----[ Stream manipulators ]---------------------------------------------------
    //! Output colour codes from CLIAttr enums.
    inline std::ostream& operator<<(std::ostream &os, CLIAttr code)
    {
        return os << CLIAttrCodes[static_cast<uint8_t>(code)];
    }

    inline std::ostream& operator<<(std::ostream &os, NodeType type)
    {
        return os << NodeTypeName[static_cast<size_t>(type)];
    }

    //! Output status messages from Status enums.
    inline std::ostream& operator<<(std::ostream &os, Status status)
    {
        switch(status) {
            case Status::Succeed:
                return os << CLIAttr::Green << "[ PASS ] " << CLIAttr::Reset;
            case Status::Fail:
                return os << CLIAttr::Red << "[ FAIL ] " << CLIAttr::Reset;
            case Status::Null:
            default:
                return os;
        }
    }

//----[ Misc functions ]--------------------------------------------------------
    /**
     * @brief Split a string on `STRING_DELIMITER` and insert the resulting
     * strings into the passed vector.
     */
    static void split_string(std::string string, std::vector<std::string> &vec)
    {
        std::string::size_type pos = 0, last = 0;
        do {
            pos = string.find_first_of(';', last);
            if(pos == std::string::npos)
            {
                pos = string.length();
                if(pos != last) vec.emplace_back(string.data() + last, pos - last);
                break;
            }
            else
            {
                if(pos != last) vec.emplace_back(string.data() + last, pos - last);
            }
            last = pos + 1;
            while(string[last] == ' ') ++last;
        } while(pos != std::string::npos);
    }
//----[ TestNode ]--------------------------------------------------------------
    struct TestNode {
        TestNode(NodeType type, Status status, std::string expr, uint32_t line, std::string str_tags = "", payload_fn fn = nullptr) :
            payload{fn},
            expr{expr},
            tags{},
            line{line},
            type{type},
            status{status},
            new_run{false},
            children_{}
        {
            if(is_container(type) && !str_tags.empty())
            {
                split_string(str_tags, tags);
            }
            else
            {
                tags.emplace_back(str_tags);
            }
            if(expr == "Anonymous Node" || expr == "")
            {
                if(type == NodeType::Section)
                {
                    this->expr = "Anonymous section (line " + std::to_string(line) + ")";
                }
                else
                {
                    this->expr = "Anonymous block (line " + std::to_string(line) + ")";
                }
            }
        }

        void push_child(uint32_t index) { children_.emplace_back(index); }
        auto begin() const -> std::vector<uint32_t>::const_iterator { return children_.begin(); }
        auto end() const -> std::vector<uint32_t>::const_iterator { return children_.end(); }
        bool empty() const { return children_.empty(); }

        payload_fn                  payload;    //<! The test function, if this is a test block.
        std::string                 expr;       //<! The name or string representation of the test expression.
        std::vector<std::string>    tags;       //<! A list of tags for this block or section.
        uint32_t                    line;       //<! The line this TestNode was invoked from.
        NodeType                    type;       //<! The type of this TestNode.
        Status                      status;     //<! Whether or not the test expression evaluated true.
        bool                        new_run;    //<! If this node is the beginning of a new run.
    private:
        std::vector<uint32_t>       children_;  //<! Indices of child nodes within the TestReferee.
    };

//----[ Test Referee ]----------------------------------------------------------
    class TestReferee {
        using NodePredicate = bool(*)(const TestNode &);
        enum class TagMatchMode {
            None,
            Any,
            All
        };
    public:
        TestReferee() :
            nodes_{},
            filter_tags_{},
            node_stack_{},
            section_stack_{},
            next_section_{},
            level_{0},
            tag_match_mode_{TagMatchMode::None},
            expand_all_{false},
            exiting_{false},
            verbose_{false}
        {
        }

        /** Initialise the TestReferee with command line arguments. Failure will
         *  return an ExprResult object containing an error message.
         */
        ExprResult init(int argc, const char *argv[])
        {
            if(argc == 1) return {true, ""};
            int curr = 1;
            do {
                std::string arg{argv[curr]};
                if(arg.length() == 2)
                {
                    if(arg[1] == 't' || arg[1] == 'x')
                    {
                        if(curr + 1 == argc || argv[curr + 1][0] == '-')
                        {
                            return {false, "No tags specified!"};
                        }
                        else if(tag_match_mode_ != TagMatchMode::None)
                        {
                            return {false, "Cannot mix -t and -x!"};
                        }
                        split_string(argv[++curr], filter_tags_);
                    }
                    switch(arg[1])
                    {
                        case 'e':
                            expand_all_ = true;
                            verbose_ = true;
                            break;
                        case 'h':
                            return {false, ""};
                            break;
                        case 't':
                            tag_match_mode_ = TagMatchMode::Any;
                            break;
                        case 'v':
                            verbose_ = true;
                            break;
                        case 'x':
                            tag_match_mode_ = TagMatchMode::All;
                            break;
                        default:
                            return {false, arg + " is not a valid argument!"};
                            break;
                    }
                }
                else
                {
                    return {false, arg + " is not a valid argument!"};
                }
                ++curr;
            } while(curr < argc);
            return {true, ""};
        }

        //! Evaluate each test case, then print the results to stdout.
        int run_tests()
        {
            NodePredicate predicate = verbose_ ? any_node : node_failed;
            const uint32_t num_blocks = evaluate_blocks();

            if(num_blocks == 0)
            {
                std::cout << "No test blocks found!" << std::endl;
                return EXIT_FAILURE;
            }

            const auto end = nodes_.begin() + num_blocks;
            for(auto block = nodes_.begin(); block != end; ++block)
            {
                if(!block->empty() && predicate(*block))
                {
                    describe_node(*block);
                    enumerate_children(*block, predicate);
                    std::cout << std::endl << std::endl;
                }
            }
            // list failed tests
            const auto num_tests = std::count_if(nodes_.begin(), nodes_.end(), is_test);
            const auto num_failed = std::count_if(nodes_.begin(), nodes_.end(), is_failed_test);
            const auto blocks_failed = std::count_if(nodes_.begin(), nodes_.end(), is_failed_block);

            if(num_failed == 0)
            {
                std::cout << "------------------ " << Status::Succeed << "------------------" << std::endl << std::endl;
                std::cout << CLIAttr::Green << "All tests passed." << std::endl;
                std::cout << "All " << num_blocks << " blocks completed successfully." << std::endl;
                std::cout << CLIAttr::Reset << std::endl;
                return EXIT_SUCCESS;
            }
            else
            {
                std::cout << "------------------ " << Status::Fail << "------------------" << std::endl << std::endl;
                std::cout << CLIAttr::Green << num_tests - num_failed << " tests passed.";
                std::cout << CLIAttr::Reset << " | " << CLIAttr::Red << num_failed << " tests failed." << std::endl;

                std::cout << CLIAttr::Green << num_blocks - blocks_failed << " blocks passed.";
                std::cout << CLIAttr::Reset << " | " << CLIAttr::Red << blocks_failed << " blocks failed." << std::endl;
                std::cout << CLIAttr::Reset << std::endl;
                return EXIT_FAILURE;
            }
            // you may ask yourself, well, how did i get here?
            return EXIT_FAILURE;
        }

        //! Run the payload and update the status for each block.
        uint32_t evaluate_blocks()
        {
            if(!filter_tags_.empty())
            {
                nodes_.erase(std::remove_if(nodes_.begin(),
                                            nodes_.end(),
                                            [this](const TestNode &node){
                                                return !matches_tags(node);
                                            }),
                                            nodes_.end());
            }
            const auto num_blocks = nodes_.size();
            for(decltype(nodes_.size()) node_id = 0; node_id < num_blocks; ++node_id)
            {
                TestNode &block = nodes_[node_id];
                node_stack_.clear();
                node_stack_.emplace_back(node_id);
                try
                {
                    run_block(block.payload, node_id);
                }
                catch(...)
                {
                    push_exception(std::current_exception(), NodeType::ThrowsOutOfNode, Status::Fail, "", nodes_.back().line);
                }
            }
            for(decltype(nodes_.size()) node_id = 0; node_id < num_blocks; ++node_id)
            {
                set_block_status(nodes_[node_id]);
            }
            return num_blocks;
        }

        void push_node(NodeType type, Status status, std::string expr, uint32_t line, std::string tags = "", payload_fn fn = nullptr, bool no_push = false)
        {
            const uint32_t node_id = nodes_.size();
            if(!node_stack_.empty())
            {
                nodes_[node_stack_.back()].push_child(node_id);
            }
            nodes_.emplace_back(type, status, expr, line, tags, fn);
            if(is_container(type) && !no_push)
            {
                node_stack_.emplace_back(node_id);
            }
        }

        void push_exception(std::exception_ptr excep, NodeType type, Status status, std::string expr_str, uint32_t line)
        {
            push_node(type, status, rethrow_get_info(excep), line, expr_str);
        }

        void run_block(void (*block)(), uint32_t node_id)
        {
            block();

            while(!next_section_.empty())
            {
                node_stack_.clear();
                node_stack_.emplace_back(node_id);
                const uint32_t section_end = nodes_.size() - 1;
                exiting_ = false;
                level_ = 0;
                block();
                nodes_[section_end + 1].new_run = true;
            }
        }

        void draw_indent(const char *indent_str) const
        {
            for(int i = 0; i < level_ ; ++i)
            {
                std::cout << indent_str;
            }
        }

        //! Print the details of a TestNode.
        void describe_node(const TestNode &node) const
        {
            if(node.type == NodeType::Section)
            {
                draw_indent("  ");
            }
            else
            {
                draw_indent("  ");
                if(level_ != 0) std::cout << "| ";
            }

            switch(node.type)
            {
                case NodeType::Block:
                    std::cout <<  CLIAttr::Bold << "----------[ ";
                    std::cout << node.expr << " ]----------";
                    break;
                case NodeType::Section:
                    std::cout << "\\- ";
                    std::cout << ((node.status == Status::Succeed) ? CLIAttr::Green : CLIAttr::Red);
                    std::cout << CLIAttr::Bold << node.expr;
                    break;
                case NodeType::Fail:
                    std::cout << node.status;
                    std::cout << CLIAttr::Red << "FAIL invoked" << CLIAttr::Reset;
                    if(!node.expr.empty()) {
                        std::cout << ": " << node.expr;
                    }
                    break;
                case NodeType::Pass:
                    std::cout << node.status;
                    std::cout << CLIAttr::Green << "SUCCEED invoked" << CLIAttr::Reset;
                    if(!node.expr.empty()) {
                        std::cout << ": " << node.expr;
                    }
                    break;
                case NodeType::Notice:
                    std::cout << CLIAttr::Reset << "NOTICE: " << node.expr;
                    break;
                case NodeType::Warn:
                    std::cout << CLIAttr::Red << "[ WARN ] " << node.expr;
                    std::cout << CLIAttr::Reset;
                    break;
                case NodeType::Throws:
                case NodeType::ThrowsAs:
                    std::cout << node.status;
                    std::cout << "with expression " << node.type;
                    std::cout << "(" << node.tags[0] << ") throwing \'" << node.expr << "\'";
                    std::cout << ", line "<< node.line;
                    break;
                case NodeType::NoThrow:
                    std::cout << node.status;
                    std::cout << "without exception '" << node.expr;
                    std::cout << "', line "<< node.line;
                    break;
                case NodeType::ThrowsUnexpected:
                    std::cout << node.status;
                    std::cout << "with unexpected exception '" << node.expr;
                    std::cout << "', line "<< node.line;
                    break;
                case NodeType::ThrowsOutOfNode:
                    std::cout << node.status;
                    std::cout << "with unexpected exception '" << node.expr;
                    std::cout << "', at some point after line "<< node.line;
                    break;
                default:
                    std::cout << node.status;
                    std::cout << "with expression " << node.type;
                    std::cout << "(" << node.tags[0];
                    std::cout << "), line "<< node.line;
                    if(node.status == Status::Fail || expand_all_) {
                        std::cout << std::endl;
                        draw_indent("  ");
                        if(level_ != 0){
                            std::cout << "| ";
                        }
                        std::cout << "                         '" << node.expr;
                        std::cout << "'";
                    }
                    break;
            }
            std::cout << CLIAttr::Reset << std::endl;
        }


        //! Rethrow an exception_ptr and display the details for that exception.
        std::string rethrow_get_info(std::exception_ptr excep) const
        {
            try
            {
                if(excep)
                {
                    std::rethrow_exception(excep);
                }
            }
            catch(const std::exception &e)
            {
                return e.what();
            }
            catch(...)
            {
                return "unknown exception";
            }
            return "no exception";
        }

        /**  Push a section into the section stack, and increase the level. If
         *   we're entering this section to navigate to a leaf node section,
         *   push a TestNode for the section. If we're exiting from a leaf
         *   node, store this as the next leaf node to be executed.
         */
        bool push_section(uint32_t line_number, std::string name)
        {
            section_stack_.emplace_back(line_number);
            const bool must_descend_further = static_cast<size_t>(level_) >= next_section_.size();
            const bool reached_leaf_node = static_cast<size_t>(level_) < next_section_.size() &&
                                           next_section_[level_] == line_number;
            if(!exiting_ &&
               (next_section_.empty() || must_descend_further || reached_leaf_node))
            {
                push_node(NodeType::Section, Status::Null, name, line_number);
                ++level_;
                return true;
            }
            else if(exiting_ && next_section_.empty())
            {
                next_section_.reserve(section_stack_.size());
                std::copy(section_stack_.begin(), section_stack_.end(), std::back_inserter(next_section_));
            }
            ++level_;
            return false;
        }

        /**  Pop a section from the section stack, and decrease the level. If
         *   we're exiting from a leaf node, clear the next_section_ record
         *   and set the exiting_ flag.
         */
        void pop_section(bool can_run)
        {
            section_stack_.pop_back();
            --level_;
            if(can_run && !exiting_)
            {
                next_section_.clear();
                exiting_ = true;
            }
        }
    private:
        static bool is_test(const TestNode &node) { return node.type <= NodeType::Pass; }
        static bool is_failed_block(const TestNode &node) { return node.type == NodeType::Block && node.status == Status::Fail; }
        static bool is_failed_test(const TestNode &node) { return node.type <= NodeType::Pass && node.status == Status::Fail; }
        static bool node_failed(const TestNode &node) { return node.status == Status::Fail; }
        static bool any_node(const TestNode &) { return true; }

        /** Describe and enumerate the children of any node fitting the
         *  supplied predicate.
         */
        void enumerate_children(const TestNode &node, NodePredicate predicate)
        {

            for(auto iter = node.begin(); iter != node.end(); ++iter)
            {
                auto &child_node = nodes_[*iter];
                if(predicate(child_node))
                {
                    if(child_node.new_run)
                    {
                        std::cout << std::endl;
                        level_ = 0;
                    }

                    describe_node(child_node);
                    if(child_node.type == NodeType::Section)
                    {
                        ++level_;
                    }

                    enumerate_children(child_node, predicate);
                    if(child_node.type == NodeType::Section)
                    {
                        --level_;
                    }

                }
            }
        }

        /** Return true if a node matches the stored tags, based on the current
         *  TagMatchMode.
         */
        bool matches_tags(const TestNode &node)
        {
            if(node.tags.empty()) return false;
            if(tag_match_mode_ == TagMatchMode::Any)
            {
                for(const auto &tag : filter_tags_)
                {
                    if(std::find(node.tags.begin(), node.tags.end(), tag) != node.tags.end())
                    {
                        return true;
                    }
                }
                return false;
            }
            else
            {
                for(const auto &tag : filter_tags_)
                {
                    if(std::find(node.tags.begin(), node.tags.end(), tag) == node.tags.end())
                    {
                        return false;
                    }
                }
                return true;
            }
        }

        /** Get the status for a block of tests based on whether or not
         *  any of its children have failed.
         */
        void set_block_status(TestNode &container)
        {
            for(const auto &child : container)
            {
                if(nodes_[child].type == NodeType::Section)
                {
                    set_block_status(nodes_[child]);
                }
            }

            container.status = Status::Succeed;
            for(const auto &child : container)
            {
                if(nodes_[child].status == Status::Fail)
                {
                    container.status = Status::Fail;
                    return;
                }
            }
        }

        std::vector<TestNode>       nodes_;             //<! List of cases, sections and assertions.
        std::vector<std::string>    filter_tags_;       //<! Tags to filter cases on.
        std::vector<uint32_t>       node_stack_;        //<! Stack of nodes for determining status inheritance.
        std::vector<uint32_t>       section_stack_;     //<! Stack of sections indicating path through current case.
        std::vector<uint32_t>       next_section_;      //<! Path to next active section.

        int                         level_;             //<! Nested section depth.
        TagMatchMode                tag_match_mode_;    //<! Current tag-matching mode.
        bool                        expand_all_;        //<! Whether or not to expand all expressions.
        bool                        exiting_;           //<! Indicates movement out of an active section.
        bool                        verbose_;           //<! Verbose mode flag.
    };

    inline TestReferee& g_TestReferee()
    {
        static TestReferee global_test_referee;
        return global_test_referee;
    }

    struct BlockLoader {
        BlockLoader(NodeType type, Status status, payload_fn fn, const NameTags &data, uint32_t line)
        {
             g_TestReferee().push_node(type, status, data.name, line, data.tags, fn, true);
        }
    };

    struct SectionLoader {
        SectionLoader(uint32_t line_number, NameTags name_tags) :
            can_run_{cataplasm::g_TestReferee().push_section(line_number, name_tags.name)},
            name_{name_tags.name},
            line_{line_number}
        {
        }
        ~SectionLoader()
        {
            g_TestReferee().pop_section(can_run_);
        }
        operator bool() const { return can_run_; }

        bool        can_run_;
        std::string name_;
        uint32_t    line_;
    };

    inline void print_help(const char *exe_name, const std::string &msg)
    {
        if(!msg.empty()) {
            std::cout << CLIAttr::Bold << "ERROR: " << msg;
            std::cout << CLIAttr::Reset << std::endl;
        }
        std::cout << std::endl << "USAGE:" << std::endl;
        std::cout << exe_name << " [-h] [-t|-x TAGS] [-v]" << std::endl;
        std::cout << std::endl << "Arguments:" << std::endl;
        std::cout << "\t-h        Prints this help message.\n";
        std::cout << "\t-e        Expand all expressions (also enables verbose mode).\n";
        std::cout << "\t-t TAGS   Run only test blocks having any of the specified tags (semicolon-separated list).\n";
        std::cout << "\t-x TAGS   Run only test blocks having all of the specified tags (semicolon-separated list).\n";
        std::cout << "\t-v        Use verbose mode, printing the results of all tests.\n";

    }
}


namespace cpm = cataplasm;
#ifdef CATAPLASM_MAIN
int main(int argc, const char **argv)
{
    cataplasm::ExprResult result = cataplasm::g_TestReferee().init(argc, argv);
    if(!result.status)
    {
        cataplasm::print_help(argv[0], result.expr);
        return EXIT_FAILURE;
    }
    return cataplasm::g_TestReferee().run_tests();
}
#endif

