template<typename T>
class ObjectPool
{
public:
    class ScopeExit
    {
    public:
        ScopeExit( ObjectPool<T> &op ) : op_{op}, obj_{op.acquire().get()} {}

        ~ScopeExit() { op_.add( std::move( obj_ ) ); }

        T &Get() { return *obj_; }

    private:
        ObjectPool<T> &op_;
        std::unique_ptr<T> obj_;
    };

    void add( std::unique_ptr<T> toAdd )
    {
        std::lock_guard<decltype( mutex_ )> lock{mutex_};
        if ( !waitQue_.empty() )
        {
            auto waiter = std::move( waitQue_.front() );
            waitQue_.pop_front();
            waiter->set_value( std::move( toAdd ) );
        }
        else
        {
            pool_.emplace( std::move( toAdd ) );
        }
    }

private:
    std::future<std::unique_ptr<T>> acquire()
    {
        std::lock_guard<decltype( mutex_ )> lock{mutex_};
        if ( pool_.empty() )
        {
            waitQue_.emplace_back( std::make_unique<std::promise<std::unique_ptr<T>>>() );
            return waitQue_.back()->get_future();
        }
        else
        {
            std::promise<std::unique_ptr<T>> p;
            p.set_value( std::move( pool_.top() ) );
            pool_.pop();
            return p.get_future();
        }
    }

    std::mutex mutex_;
    std::stack<std::unique_ptr<T>> pool_;
    std::deque<std::unique_ptr<std::promise<std::unique_ptr<T>>>> waitQue_;
};
