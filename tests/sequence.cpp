class Sequence
{
public:
    Sequence()
    {
        this->id = 0;
    }

    int GetNextID()
    {
        return ++this->id;
    }
private:
    int id;
};
