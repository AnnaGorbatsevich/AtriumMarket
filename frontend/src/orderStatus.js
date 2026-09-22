export const STATUS_LABELS = {
  ordered: 'Оформлен',
  accepted: 'Принят',
  shipped: 'Передан в доставку',
  ready_for_pickup: 'Доставлен',
  received: 'Получен',
  cancelled: 'Отменён',
};

// What each side may do with an order in a given status. This only decides which buttons to show;
// order_service enforces the same rules.
export const SELLER_ACTIONS = {
  ordered: [
    { status: 'accepted', label: 'Принять' },
    { status: 'cancelled', label: 'Отменить', danger: true },
  ],
  accepted: [{ status: 'shipped', label: 'Передать в доставку' }],
  shipped: [{ status: 'ready_for_pickup', label: 'Товар доставлен' }],
};

export const BUYER_ACTIONS = {
  ready_for_pickup: [
    { status: 'received', label: 'Забрать товар' },
    { status: 'cancelled', label: 'Отказаться', danger: true },
  ],
};

// Shown when the current side has nothing to do with an order that is not finished yet.
export const SELLER_WAITING = {
  ready_for_pickup: 'Ждём, пока покупатель заберёт товар',
};

export const BUYER_WAITING = {
  ordered: 'Ждём подтверждения продавца',
  accepted: 'Продавец готовит заказ к отправке',
  shipped: 'Заказ в пути',
};
